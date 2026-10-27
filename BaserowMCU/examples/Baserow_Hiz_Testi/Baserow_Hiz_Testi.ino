// BaserowMCU ile cift yonlu IoT kontrol ve haberlesme suresi olcumu.
// Kullanici ornegi: mikrodunya.wordpress.com
// Donanim: ESP32-C3, GPIO 8 NeoPixel. Baska kartta LED_PIN'i uyarlayin.
#include <BaserowMCU.h> // ESP32 / ESP8266 WiFi basliklari otomatik secilir.
#include <Adafruit_NeoPixel.h>
#include "secrets.h"

#define LED_PIN 8
#define NUMPIXELS 1

// Karsilastirmada SADECE bir ayari degistirip yeniden yukleyin.
const bool TOPLU_YAZMA = true;       // true: iki alan tek PATCH; false: iki PATCH.
const bool KALICI_BAGLANTI = true;   // false: her istekte yeni baglanti.
const int ROW_ID = 1;               // Gercek Baserow satir ID'si, gorunen sira degil.
const uint32_t ARA_BEKLEME_MS = 1000;
const uint32_t TUR_SONU_BEKLEME_MS = 1000;

BaserowMCU baserow(BASEROW_TABLE_ID, BASEROW_TOKEN);
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
byte ledDurum = 0;
uint32_t turSayisi = 0;
uint32_t istekSayisi = 0;
uint32_t basariliTur = 0;
uint32_t hataliTur = 0;
uint64_t toplamBasariliMs = 0;
uint32_t enAzMs = UINT32_MAX;
uint32_t enCokMs = 0;

void led_yak() {
  pixels.setPixelColor(0, pixels.Color(120, 0, 80));
  pixels.show();
  ledDurum = 1;
}
void led_sondur() {
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
  ledDurum = 0;
}

// Sureyi Serial ciktilarindan ONCE alir. API suresi JSON islemeyi de kapsar.
// Kutuphane HTTP suresi JSON hazirlama/ayristirmayi kapsamaz; API suresinin alt kumesidir.
uint32_t olcumuYaz(const char* ad, uint32_t baslangic, bool basarili) {
  const uint32_t gecenMs = millis() - baslangic;
  ++istekSayisi;
  Serial.printf("[ISTEK %lu] %s | %s | API=%lu ms | HTTP=%lu ms | kod=%d | yanit=%u bayt\n",
      static_cast<unsigned long>(istekSayisi), ad, basarili ? "OK" : "HATA",
      static_cast<unsigned long>(gecenMs),
      static_cast<unsigned long>(baserow.lastRequestMs()), baserow.lastHttpCode(),
      static_cast<unsigned int>(baserow.lastResponseBytes()));
  if (!basarili) Serial.printf("  Aciklama: %s\n", baserow.lastError().c_str());
  return gecenMs;
}

void setup() {
  Serial.begin(115200);
  pixels.begin();
  led_sondur();
  delay(100);

  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  const uint32_t wifiBaslangic = millis();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  const uint32_t wifiMs = millis() - wifiBaslangic;

  baserow.setKeepAlive(KALICI_BAGLANTI);
  baserow.setReadCacheMs(0); // Onbellek hizi degil, gercek ag islemini olcuyoruz.
  Serial.printf("\nPlatform=%s | WiFi baglanma=%lu ms | RSSI=%ld dBm\n",
      BaserowMCU::platform(), static_cast<unsigned long>(wifiMs), static_cast<long>(WiFi.RSSI()));
  Serial.printf("Toplu yazma=%s | Kalici baglanti=%s | Row ID=%d\n",
      TOPLU_YAZMA ? "ACIK" : "KAPALI", KALICI_BAGLANTI ? "ACIK" : "KAPALI", ROW_ID);
  Serial.println("Ilk istek yeni TLS baglantisini icerir; sonraki turlari da karsilastirin.");
  Serial.println("API sureleri beklemeleri icermez. HTTP suresi API suresine zaten dahildir.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    baserow.close();
    Serial.println("WiFi baglantisi yok; bu tur olculmedi.");
    delay(1000);
    return;
  }
  ++turSayisi;
  Serial.printf("\n--- TUR %lu | RSSI=%ld dBm ---\n",
      static_cast<unsigned long>(turSayisi), static_cast<long>(WiFi.RSSI()));
  const uint32_t turBaslangic = millis();
  const float sicaklik = random(0, 1000) / 10.0f;
  uint32_t yazmaMs = 0;
  bool yazmaOK = false;

  // Sicaklik ve onceki komuttan kalan LED geri bildirimi gonderilir.
  if (TOPLU_YAZMA) {
    JsonDocument fields;
    fields["Sicaklik"] = sicaklik;
    fields["Led_Durum"] = ledDurum;
    const uint32_t t = millis();
    yazmaOK = baserow.updateRow(ROW_ID, fields);
    yazmaMs = olcumuYaz("PATCH Sicaklik + Led_Durum", t, yazmaOK);
  } else {
    uint32_t t = millis();
    const bool sicaklikOK = baserow.updateNumber(ROW_ID, "Sicaklik", sicaklik);
    yazmaMs += olcumuYaz("PATCH Sicaklik", t, sicaklikOK);
    t = millis();
    const bool durumOK = baserow.updateNumber(ROW_ID, "Led_Durum", ledDurum);
    yazmaMs += olcumuYaz("PATCH Led_Durum", t, durumOK);
    yazmaOK = sicaklikOK && durumOK;
  }

  uint32_t beklemeBaslangic = millis();
  delay(ARA_BEKLEME_MS);
  uint32_t beklemeMs = millis() - beklemeBaslangic;

  // Yazilan satirin ID'siyle okur; eski readNumber'in liste sirasi kullanilmaz.
  JsonDocument row;
  const uint32_t okumaBaslangic = millis();
  const bool okumaOK = baserow.readRow(ROW_ID, row);
  const uint32_t okumaMs = olcumuYaz("GET Led_Komut (satir)", okumaBaslangic, okumaOK);
  if (okumaOK && !row["Led_Komut"].isNull()) {
    const float ledEmri = row["Led_Komut"].as<float>();
    if (ledEmri == 1.0f) {
      led_yak();
      Serial.println("LED ACILDI.");
    } else if (ledEmri == 0.0f) {
      led_sondur();
      Serial.println("LED KAPATILDI.");
    } else {
      Serial.println("Led_Komut 0 veya 1 degil; LED durumu korundu.");
    }
  } else if (okumaOK) {
    Serial.println("Led_Komut alani yok veya bos; LED durumu korundu.");
  }

  beklemeBaslangic = millis();
  delay(TUR_SONU_BEKLEME_MS);
  beklemeMs += millis() - beklemeBaslangic;
  const uint32_t turMs = millis() - turBaslangic;
  const uint32_t apiMs = yazmaMs + okumaMs;
  Serial.printf("[TUR] Yazma=%lu ms | Okuma=%lu ms | API toplam=%lu ms | Bekleme=%lu ms | Tur=%lu ms\n",
      static_cast<unsigned long>(yazmaMs), static_cast<unsigned long>(okumaMs),
      static_cast<unsigned long>(apiMs), static_cast<unsigned long>(beklemeMs),
      static_cast<unsigned long>(turMs));

  if (yazmaOK && okumaOK) {
    // Ilk tur soguk baglanti olarak ayri tutulur; ortalamaya katilmaz.
    if (turSayisi == 1) {
      Serial.printf("[ILK TUR] API toplam=%lu ms; ortalamaya katilmadi.\n",
          static_cast<unsigned long>(apiMs));
    } else {
      ++basariliTur;
      toplamBasariliMs += apiMs;
      if (apiMs < enAzMs) enAzMs = apiMs;
      if (apiMs > enCokMs) enCokMs = apiMs;
    }
  } else {
    ++hataliTur;
    Serial.println("Bu turda HTTP hatasi var; basarili tur ortalamasina katilmadi.");
  }
  if (basariliTur) {
    Serial.printf("[OZET] Basarili tur=%lu | API ort=%.1f ms | min=%lu ms | max=%lu ms | Hatali tur=%lu\n",
        static_cast<unsigned long>(basariliTur), static_cast<double>(toplamBasariliMs) / basariliTur,
        static_cast<unsigned long>(enAzMs), static_cast<unsigned long>(enCokMs),
        static_cast<unsigned long>(hataliTur));
  }
}
