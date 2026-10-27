#ifndef BASEROW_MCU_H
#define BASEROW_MCU_H

#include <Arduino.h>
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
  #define BASEROW_MCU_ESP8266 1
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  #define BASEROW_MCU_ESP32 1
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #error "BaserowMCU requires an ESP32 or ESP8266 board."
#endif
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <memory>
#if ARDUINOJSON_VERSION_MAJOR < 7
  #error "BaserowMCU requires ArduinoJson 7 or newer."
#endif

class BaserowMCU {
 public:
  BaserowMCU(String tableId, String apiToken,
             String baseUrl = "https://api.baserow.io");
  BaserowMCU(const BaserowMCU&) = delete;
  BaserowMCU& operator=(const BaserowMCU&) = delete;
  static const char* platform();

  // Original API: reads use 1-based API list POSITION; updates use actual row ID.
  bool insertNumber(String fieldName, float value);
  bool insertText(String fieldName, String text);
  bool updateNumber(int rowId, String fieldName, float value);
  bool updateText(int rowId, String fieldName, String text);
  float readNumber(String fieldName, int rowNumber = 1);
  String readText(String fieldName, int rowNumber = 1);

  // One request for all fields. Read by actual row ID (not display position).
  bool readRow(int rowId, JsonDocument& result);
  bool insertRow(const JsonDocument& fields);
  bool updateRow(int rowId, const JsonDocument& fields);

  void setTimeout(uint16_t timeoutMs);
  void setKeepAlive(bool enabled);
  void setReadCacheMs(uint32_t ttlMs); // Opt-in; default 0 (fresh reads).
  void clearCache();
  void close();
  void setMaxResponseSize(size_t bytes); // Default 16384 for JSON reads.
  void setCACert(const char* pem); // Copies PEM; sync device clock first.
  void setInsecure(); // Compatibility default; no server identity verification.
  int lastHttpCode() const { return _lastHttpCode; }
  const String& lastError() const { return _lastError; }
  uint32_t lastRequestMs() const { return _lastRequestMs; }
  size_t lastResponseBytes() const { return _lastResponseBytes; }
  bool lastReadFromCache() const { return _lastReadFromCache; }

 private:
  String _baseUrl, _rowsUrl, _authorization;
  // Dependencies outlive client; HTTPClient is destroyed before client.
  String _caPem;
#if defined(BASEROW_MCU_ESP8266)
  BearSSL::Session _session;
  std::unique_ptr<BearSSL::X509List> _trustAnchors;
#endif
  WiFiClientSecure _client;
  HTTPClient _http;
  bool _keepAlive = true;
  uint16_t _timeoutMs = 5000;
  size_t _maxResponseSize = 16384;
  int _lastHttpCode = 0;
  String _lastError;
  uint32_t _lastRequestMs = 0;
  size_t _lastResponseBytes = 0;
  bool _lastReadFromCache = false;
  uint32_t _cacheTtlMs = 0, _cacheTime = 0;
  int _cachePosition = 0;
  JsonDocument _cache;

  void resetStatus();
  bool fail(const char* message);
  String rowUrl(int rowId) const;
  bool request(const char* method, const String& url,
               const String* payload, String* response);
  bool writeRow(const char* method, const String& url, const JsonDocument& fields);
  bool fetchPosition(int rowNumber);
};
#endif
