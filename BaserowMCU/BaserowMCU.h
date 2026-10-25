#ifndef BASEROW_MCU_H
#define BASEROW_MCU_H

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

class BaserowMCU {
  public:
    BaserowMCU(String tableId, String apiToken, String baseUrl = "https://baserow.io");
    
    // Veri gönderme fonksiyonları
    bool insertNumber(String fieldName, float value);
    bool insertText(String fieldName, String text);

    // Belirli bir satırı güncelleme fonksiyonları
    bool updateNumber(int rowId, String fieldName, float value);
    bool updateText(int rowId, String fieldName, String text);

    // Veri okuma fonksiyonları (Baserow arayüzündeki satır numarası girilir: 1, 2, 3...)
    float readNumber(String fieldName, int rowNumber = 1);
    String readText(String fieldName, int rowNumber = 1);

  private:
    String _baseUrl;
    String _tableId;
    String _rowsUrl;
    String _apiToken;
    String getRawJson();
    bool sendRawJson(String& jsonPayload, String targetUrl, String method);
};

#endif
