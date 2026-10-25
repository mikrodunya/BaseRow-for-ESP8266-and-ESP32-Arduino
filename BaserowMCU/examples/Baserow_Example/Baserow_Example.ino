#include <WiFi.h> // ESP8266 için: #include <ESP8266WiFi.h>
#include <BaserowMCU.h>

// 🔒 KODDA HİÇBİR YERDE GERÇEK ŞİFRENİZ BARINMAZ (Yüklemeden önce doldurun)
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Baserow Bağlantısı (Table ID ve Database Token)
BaserowMCU baserow("YOUR_TABLE_ID", "YOUR_DATABASE_TOKEN");

const int LED_PIN = 2; // Durum kontrol LED'i

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  WiFi.disconnect(true); // Önceki hatalı bağlantı kalıntılarını temizle
  delay(1000);
  WiFi.mode(WIFI_STA);   // Sadece Station (İstemci) modu aktif
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi Bağlantısı Başarılı!");
}

void loop() {
  float anlikRuzgar = 12.4 + (random(0, 100) / 10.0);
  String anlikDurum = (anlikRuzgar > 18.0) ? "FIRTINA" : "NORMAL";

  Serial.println("\n==================================================");
  Serial.println("📂 YENİ SATIR EKLEME");
  Serial.println("==================================================");
  
  if (baserow.insertNumber("Ruzgar_Hizi", anlikRuzgar)) {
    Serial.println("-> Başarılı: Yeni rüzgar hızı satırı eklendi.");
  }
  if (baserow.insertText("Hava_Kalitesi", anlikDurum + "_LOG")) {
    Serial.println("-> Başarılı: Yeni günlük metin satırı eklendi.");
  }

  delay(2000);

  Serial.println("\n==================================================");
  Serial.println("📂 SABİT SATIR GÜNCELLEME");
  Serial.println("==================================================");
  
  // Baserow panelindeki 1. satırı günceller
  if (baserow.updateNumber(1, "Ruzgar_Hizi", anlikRuzgar)) {
    Serial.println("-> Başarılı: ID:1 olan satırdaki rüzgar hızı güncellendi.");
  }
  if (baserow.updateText(1, "Hava_Kalitesi", anlikDurum)) {
    Serial.println("-> Başarılı: ID:1 olan satırdaki durum metni güncellendi.");
  }

  delay(2000);

  Serial.println("\n==================================================");
  Serial.println("📂 VERİ OKUMA");
  Serial.println("==================================================");

  // 🎯 YENİLİK: Direkt "1" yazarak Baserow'daki 1. satırı okuyoruz!
  float ledEmri = baserow.readNumber("LED_Komut", 1); 
  if (ledEmri == 1.0) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("-> readNumber() Sonucu: LED AÇILDI.");
  } else if (ledEmri == 0.0) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("-> readNumber() Sonucu: LED KAPATILDI.");
  }

  // 🎯 YENİLİK: Direkt "1" yazarak Baserow'daki 1. satırı okuyoruz!
  String havaDurumu = baserow.readText("Hava_Kalitesi", 1);
  Serial.print("-> readText() Sonucu (Hava Durumu): ");
  Serial.println(havaDurumu);

  Serial.println("==================================================");
  delay(30000); // 30 saniye bekle
}
