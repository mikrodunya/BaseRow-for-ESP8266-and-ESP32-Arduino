#include "BaserowMCU.h"

BaserowMCU::BaserowMCU(String tableId, String apiToken, String baseUrl) {
  _tableId = tableId;
  _apiToken = apiToken;
  _baseUrl = baseUrl;
  _rowsUrl = baseUrl + "/api/database/rows/table/" + _tableId + "/?user_field_names=true";
}

bool BaserowMCU::insertNumber(String fieldName, float value) {
  JsonDocument doc; doc[fieldName] = value;
  String payload; serializeJson(doc, payload);
  return sendRawJson(payload, _rowsUrl, "POST");
}

bool BaserowMCU::insertText(String fieldName, String text) {
  JsonDocument doc; doc[fieldName] = text;
  String payload; serializeJson(doc, payload);
  return sendRawJson(payload, _rowsUrl, "POST");
}

bool BaserowMCU::updateNumber(int rowId, String fieldName, float value) {
  JsonDocument doc; doc[fieldName] = value;
  String payload; serializeJson(doc, payload);
  String url = _baseUrl + "/api/database/rows/table/" + _tableId + "/" + String(rowId) + "/?user_field_names=true";
  return sendRawJson(payload, url, "PATCH");
}

bool BaserowMCU::updateText(int rowId, String fieldName, String text) {
  JsonDocument doc; doc[fieldName] = text;
  String payload; serializeJson(doc, payload);
  String url = _baseUrl + "/api/database/rows/table/" + _tableId + "/" + String(rowId) + "/?user_field_names=true";
  return sendRawJson(payload, url, "PATCH");
}

bool BaserowMCU::sendRawJson(String& jsonPayload, String targetUrl, String method) {
  if (WiFi.getMode() != WIFI_STA || WiFi.status() != WL_CONNECTED) return false;
  
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  http.begin(client, targetUrl);
  http.addHeader("Authorization", "Token " + _apiToken);
  http.addHeader("Content-Type", "application/json");
  
  int code = 0;
  if (method == "POST") code = http.POST(jsonPayload);
  else if (method == "PATCH") code = http.PATCH(jsonPayload);
  http.end();
  
  return (code >= 200 && code < 300);
}

String BaserowMCU::getRawJson() {
  if (WiFi.getMode() != WIFI_STA || WiFi.status() != WL_CONNECTED) return "";
  
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  http.begin(client, _rowsUrl);
  http.addHeader("Authorization", "Token " + _apiToken);
  
  int code = http.GET();
  String response = (code == 200) ? http.getString() : "";
  http.end();
  return response;
}

float BaserowMCU::readNumber(String fieldName, int rowNumber) {
  // İnsan dilindeki satır numarasını yazılım indeksine çeviriyoruz (1. satır -> index 0)
  int rowIndex = rowNumber - 1;
  if (rowIndex < 0) rowIndex = 0; // Hatalı giriş koruması

  String json = getRawJson(); if (json == "") return -1.0;
  DynamicJsonDocument doc(4096); if (deserializeJson(doc, json)) return -1.0;
  
  if (doc["results"][rowIndex].containsKey(fieldName)) {
    return doc["results"][rowIndex][fieldName].as<float>();
  }
  return -1.0;
}

String BaserowMCU::readText(String fieldName, int rowNumber) {
  // İnsan dilindeki satır numarasını yazılım indeksine çeviriyoruz (1. satır -> index 0)
  int rowIndex = rowNumber - 1;
  if (rowIndex < 0) rowIndex = 0; // Hatalı giriş koruması

  String json = getRawJson(); if (json == "") return "HATA";
  DynamicJsonDocument doc(4096); if (deserializeJson(doc, json)) return "HATA";
  
  if (doc["results"][rowIndex].containsKey(fieldName)) {
    return doc["results"][rowIndex][fieldName].as<String>();
  }
  return "KOLON_BULUNAMADI";
}
