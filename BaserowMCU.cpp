#include "BaserowMCU.h"
#include <StreamString.h>
#include <utility>

namespace {
// HTTPClient decodes chunked transfer before writing here. Write responses
// are drained without allocating a second full JSON document.
class ResponseSink : public StreamString {
 public:
  ResponseSink(bool keepBody, size_t limit) : _keepBody(keepBody), _limit(limit) {}
  size_t write(uint8_t value) override { return write(&value, 1); }
  size_t write(const uint8_t* data, size_t size) override {
    if (_failed) return 0;
    if (_keepBody && (size > _limit - length() ||
        StreamString::write(data, size) != size)) {
      _failed = true;
      return 0;
    }
    return size;
  }
  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }
  void flush() override {}
  bool failed() const { return _failed; }
 private:
  bool _keepBody;
  size_t _limit;
  bool _failed = false;
};
}

BaserowMCU::BaserowMCU(String tableId, String apiToken, String baseUrl)
    : _baseUrl(baseUrl), _authorization("Token " + apiToken) {
  while (_baseUrl.endsWith("/")) _baseUrl.remove(_baseUrl.length() - 1);
  _rowsUrl = _baseUrl + "/api/database/rows/table/" + tableId + "/";
  _client.setInsecure(); // Preserve the original library's TLS policy.
#if defined(BASEROW_MCU_ESP8266)
  _client.setSession(&_session);
#endif
}

const char* BaserowMCU::platform() {
#if defined(BASEROW_MCU_ESP8266)
  return "ESP8266";
#else
  return "ESP32";
#endif
}

void BaserowMCU::resetStatus() {
  _lastHttpCode = 0;
  _lastError = "";
  _lastRequestMs = 0;
  _lastResponseBytes = 0;
  _lastReadFromCache = false;
}
bool BaserowMCU::fail(const char* message) {
  _lastError = message;
  return false;
}
void BaserowMCU::clearCache() { _cache.clear(); _cachePosition = 0; }
void BaserowMCU::setReadCacheMs(uint32_t ttlMs) { _cacheTtlMs = ttlMs; clearCache(); }
void BaserowMCU::setMaxResponseSize(size_t bytes) { _maxResponseSize = bytes; clearCache(); }
void BaserowMCU::setTimeout(uint16_t timeoutMs) { _timeoutMs = timeoutMs ? timeoutMs : 1; }
void BaserowMCU::close() {
  _http.setReuse(false);
  _http.end();
  _client.stop();
  clearCache();
}
void BaserowMCU::setKeepAlive(bool enabled) { close(); _keepAlive = enabled; }
void BaserowMCU::setInsecure() {
  close();
  _client.setInsecure();
#if defined(BASEROW_MCU_ESP8266)
  _session = BearSSL::Session();
  _trustAnchors.reset();
#endif
  _caPem = "";
}
void BaserowMCU::setCACert(const char* pem) {
  if (!pem || !pem[0]) { setInsecure(); return; }
  close();
  _caPem = pem;
#if defined(BASEROW_MCU_ESP8266)
  _session = BearSSL::Session();
  _trustAnchors.reset(new BearSSL::X509List(_caPem.c_str()));
  _client.setTrustAnchors(_trustAnchors.get());
#else
  _client.setCACert(_caPem.c_str());
#endif
}
String BaserowMCU::rowUrl(int rowId) const {
  return _rowsUrl + String(rowId) + "/?user_field_names=true";
}

bool BaserowMCU::request(const char* method, const String& url,
                         const String* payload, String* response) {
  resetStatus();
  const uint32_t started = millis();
  if (response) *response = "";
  if (WiFi.status() != WL_CONNECTED) {
    close();
    return fail("WiFi is not connected");
  }
  if (!url.startsWith("https://")) return fail("HTTPS base URL required");

  // A GET may be retried once on transport failure. Never replay a write:
  // the server might already have committed it before the connection failed.
  const uint8_t attempts = strcmp(method, "GET") == 0 ? 2 : 1;
  for (uint8_t attempt = 0; attempt < attempts; ++attempt) {
    _http.setReuse(_keepAlive);
    _http.setTimeout(_timeoutMs);
#if defined(BASEROW_MCU_ESP32)
    _http.setConnectTimeout(_timeoutMs);
#endif
    if (!_http.begin(_client, url)) {
      close();
      _lastRequestMs = millis() - started;
      return fail("HTTP begin failed");
    }
    _http.addHeader("Authorization", _authorization);
    if (payload) _http.addHeader("Content-Type", "application/json");
    _lastHttpCode = _http.sendRequest(method,
        payload ? reinterpret_cast<uint8_t*>(const_cast<char*>(payload->c_str())) : nullptr,
        payload ? payload->length() : 0);
    if (_lastHttpCode <= 0) {
      _lastError = HTTPClient::errorToString(_lastHttpCode);
      close();
      if (attempt + 1 < attempts) continue;
      _lastRequestMs = millis() - started;
      return false;
    }
    _lastError = "";
    const bool success = _lastHttpCode >= 200 && _lastHttpCode < 300;
    String* target = success ? response : nullptr;
    const int expected = _http.getSize();
    if (target && expected > 0 && static_cast<size_t>(expected) > _maxResponseSize) {
      close();
      _lastRequestMs = millis() - started;
      return fail("Response exceeds max response size");
    }
    ResponseSink sink(target != nullptr, _maxResponseSize);
    // Consume the whole body on both reads and writes before reusing TCP/TLS.
    const int received = expected == 0 ? 0 : _http.writeToStream(&sink);
    if (received < 0 || sink.failed() || (expected >= 0 && received != expected)) {
      _lastError = sink.failed() ? "Response too large or out of memory" : "Incomplete HTTP response";
      if (response) *response = "";
      close();
      _lastRequestMs = millis() - started;
      return false;
    }
    if (target) *target = std::move(static_cast<String&>(sink));
    _lastResponseBytes = static_cast<size_t>(received);
    _http.end(); // Clears headers; keeps socket only when both peers allow it.
    _lastRequestMs = millis() - started;
    if (!success) { _lastError = "HTTP " + String(_lastHttpCode); return false; }
    return true;
  }
  return false;
}

bool BaserowMCU::writeRow(const char* method, const String& url, const JsonDocument& fields) {
  resetStatus();
  if (fields.overflowed() || !fields.is<JsonObjectConst>() || fields.size() == 0)
    return fail("Expected a non-empty JSON object without allocation errors");
  String payload;
  const size_t length = measureJson(fields);
  if (serializeJson(fields, payload) != length || payload.length() != length)
    return fail("Not enough memory for request");
  clearCache(); // Also invalidate on ambiguous write failures.
  return request(method, url, &payload, nullptr);
}
bool BaserowMCU::insertRow(const JsonDocument& fields) {
  return writeRow("POST", _rowsUrl + "?user_field_names=true", fields);
}
bool BaserowMCU::updateRow(int rowId, const JsonDocument& fields) {
  if (rowId < 1) { resetStatus(); return fail("Row ID must be positive"); }
  return writeRow("PATCH", rowUrl(rowId), fields);
}
bool BaserowMCU::insertNumber(String fieldName, float value) {
  JsonDocument fields; fields[fieldName] = value; return insertRow(fields);
}
bool BaserowMCU::insertText(String fieldName, String text) {
  JsonDocument fields; fields[fieldName] = text; return insertRow(fields);
}
bool BaserowMCU::updateNumber(int rowId, String fieldName, float value) {
  JsonDocument fields; fields[fieldName] = value; return updateRow(rowId, fields);
}
bool BaserowMCU::updateText(int rowId, String fieldName, String text) {
  JsonDocument fields; fields[fieldName] = text; return updateRow(rowId, fields);
}

bool BaserowMCU::readRow(int rowId, JsonDocument& result) {
  result.clear();
  if (rowId < 1) { resetStatus(); return fail("Row ID must be positive"); }
  String response;
  if (!request("GET", rowUrl(rowId), nullptr, &response)) return false;
  DeserializationError error = deserializeJson(result, response);
  if (error || !result.is<JsonObject>() || result["id"].as<int>() != rowId) {
    result.clear();
    return fail(error ? error.c_str() : "Unexpected row response");
  }
  return true;
}

bool BaserowMCU::fetchPosition(int rowNumber) {
  if (rowNumber < 1) rowNumber = 1; // Original API compatibility.
  resetStatus();
  if (WiFi.status() != WL_CONNECTED) { close(); return fail("WiFi is not connected"); }
  if (_cacheTtlMs && _cachePosition == rowNumber &&
      static_cast<uint32_t>(millis() - _cacheTime) < _cacheTtlMs) {
    _lastReadFromCache = true;
    return true;
  }
  clearCache();
  String response;
  // Preserve positional semantics, but transfer only the requested row.
  const String url = _rowsUrl + "?user_field_names=true&size=1&page=" + String(rowNumber);
  if (!request("GET", url, nullptr, &response)) return false;
  JsonDocument document;
  DeserializationError error = deserializeJson(document, response);
  if (error) return fail(error.c_str());
  JsonObjectConst row = document["results"][0].as<JsonObjectConst>();
  if (row.isNull()) return fail("Row position not found");
  if (!_cache.set(row)) { clearCache(); return fail("Not enough memory for row"); }
  _cachePosition = rowNumber;
  _cacheTime = millis();
  return true;
}
float BaserowMCU::readNumber(String fieldName, int rowNumber) {
  if (!fetchPosition(rowNumber)) return -1.0;
  if (_cache[fieldName].isNull()) { fail("Field not found or null"); return -1.0; }
  return _cache[fieldName].as<float>();
}
String BaserowMCU::readText(String fieldName, int rowNumber) {
  if (!fetchPosition(rowNumber)) return "HATA";
  if (_cache[fieldName].isNull()) { fail("Field not found or null"); return "KOLON_BULUNAMADI"; }
  return _cache[fieldName].as<String>();
}
