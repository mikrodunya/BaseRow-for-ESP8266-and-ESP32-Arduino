# BaserowMCU 🚀

BaserowMCU, **ESP32** ve **ESP8266** tabanlı mikrodenetleyicilerinizin açık kaynaklı no-code veritabanı platformu [Baserow](https://baserow.io) ile doğrudan REST API üzerinden iletişim kurmasını sağlayan; hafif, kararlı ve kullanımı son derece pratik bir Arduino kütüphanesidir.

---

## ✨ Özellikler

- **🔢 Doğrudan Satır Okuma Desteği (v1.5.0):** Yazılım dünyasındaki 0'dan başlama (sıfır tabanlı indeks) karmaşası kaldırılmıştır. Baserow web arayüzünde gördüğünüz satır numarası (1, 2, 3...) neyse, fonksiyonlara doğrudan o numarayı yazarak veri okuyabilirsiniz.
- **📦 Sıfır JSON Karmaşası:** Ana kodunuzun içinde karmaşık JSON string birleştirmeleri, kaçış karakterleri (`\"`) veya kütüphane nesneleri görmezsiniz. Verilerinizi doğrudan `float` veya `String` olarak iletebilirsiniz.
- **🔄 Çift Yönlü İletişim (POST / PATCH / GET):** Veritabanına sadece veri yazmakla (Log kaydı tutma) kalmaz, mevcut bir satırın üzerine yazarak güncelleyebilir (Canlı panel ekranı gibi) veya tablodaki verileri geri okuyabilirsiniz.
- **🔒 Güvenli Tasarım:** Projenizi GitHub'a yüklerken Wi-Fi şifrenizi, Table ID'nizi veya API Token'ınızı kodun içine gömmek zorunda kalmazsınız. Örnek kodlar tamamen şifrelerden arındırılmıştır.
- **🛡️ Sıkı STA Modu Güvencesi:** Kütüphane, canlı ortamlarda (field) IP çakışmalarını veya kilitlenmeleri önlemek için kartı sıkı bir şekilde `WIFI_STA` (İstemci) modunda tutar ve ağ kesintilerinde otomatik askıya alma algoritmasıyla çalışır.

---

## 🔌 Donanım Uyumluluğu (Supported Hardware)

Bu kütüphane, yerleşik Wi-Fi modülü bulunan ve HTTPS (SSL/TLS) el sıkışması yapabilecek yeterli bellek kapasitesine sahip **32-bit mimariler** için tasarlanmıştır.

| Mikrodenetleyici Ailesi | Örnek Geliştirme Kartları | Wi-Fi Desteği | SSL/TLS (HTTPS) Güvenliği | Durum |
| :--- | :--- | :--- | :--- | :--- |
| **ESP32 Serisi** | ESP32-WROOM-32, ESP32-S2, ESP32-S3, ESP32-C3, NodeMCU-32S | Yerleşik (Native) | Donanımsal Kripto Hızlandırma | ✅ Tam Uyumlu |
| **ESP8266 Serisi** | NodeMCU V2/V3, WeMos D1 Mini, ESP-01S | Yerleşik (Native) | Yazılımsal Güvenlik (RAM sınırına dikkat) | ✅ Tam Uyumlu |

> ⚠️ **Önemli Not:** Arduino Uno, Nano veya Mega gibi klasik 8-bit kartlar **yerleşik Wi-Fi modülüne ve şifreli HTTPS trafiğini yönetebilecek donanıma sahip olmadıkları için** bu kütüphaneyi doğrudan desteklemez.

---

## 📚 Kütüphane Bağımlılıkları (Dependencies)

Kütüphanenin hatasız derlenebilmesi için Arduino IDE'nize şu bağımlılığın kurulması gerekir:
1. **ArduinoJson (Sürüm 7.x veya üzeri):** Baserow API'sinden dönen JSON şemalarını arka planda ayrıştırmak ve sunucuya veri paketlemek için kullanılır.
   - *Nasıl Kurulur?:* Arduino IDE > Araçlar > Kütüphaneleri Yönet (Library Manager) bölümünden **"ArduinoJson"** aratın ve Benoit Blanchon tarafından yazılan güncel sürümü kurun.

---

## 🔑 Baserow API Kimlik Bilgilerini Alma (How to Get ID & Token)

### 1. Database Token (API Anahtarı) Oluşturma
1. Baserow hesabınıza giriş yapın.
2. Sol üst köşedeki **Workspace (Çalışma Alanı)** adınıza tıklayıp **My Settings** menüsüne gidin.
3. Sol menüden **Database tokens** sekmesine tıklayın ve **Create token +** butonuna basın.
4. Bir isim verip çalışma alanınızı seçerek token'ı oluşturun.
5. **Kritik Adım:** Oluşturulan token satırının sağındaki üç noktaya (**⋮**) tıklayın, **Show databases** deyin ve ilgili veritabanınız için **Yazma ve Okuma (Write / Read)** yetkilerini kesinlikle aktifleştirin.

### 2. Table ID (Tablo Numarası) Bulma
- Tarayıcı adres çubundaki URL'de yer alan `/table/` ibaresinden sonraki sayı sizin Table ID'nizdir. 
  *(Örn: `.../database/12/table/3456/` ise Table ID **3456**'dır).*

---

## 📊 Desteklenen Veri Tipleri Özeti

| Arduino Türü | Baserow Kolon Tipi | Açıklama |
| :--- | :--- | :--- |
| `float` / `int` | **Number** (Sayı) | Sensör değerleri, sayaçlar, ondalıklı veriler. |
| `String` / `const char*` | **Text** (Metin) | Cihaz durumları, hata logları, sözel bildirimler. |
| `bool` (`true`/`false`) | **Boolean** (Checkbox) | `updateNumber` ile `1` (İşaretli) veya `0` (Boş) olarak yönetilir. |

---

## 🚀 Güvenli Örnek Kullanım (Safe Example Code)

Baserow üzerinde **`Ruzgar_Hizi` (Sayı)**, **`Hava_Kalitesi` (Metin)** ve **`LED_Komut` (Sayı)** kolonlarını elinizle oluşturun. Tablonuza test için **en az bir adet boş satır (Row ID: 1)** ekleyin. 

Aşağıdaki kodu kartınıza yüklemeden önce kendi ağ ve API bilgilerinizi `YOUR_` ile başlayan ilgili yerlere yazın. Kodu GitHub'a bu haliyle yüklediğinizde hiçbir şifreniz internete sızmaz:

```cpp
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
  Serial.println("📂 YENİ SATIR EKLEME (JSON KULLANMADAN)");
  Serial.println("==================================================");
  
  if (baserow.insertNumber("Ruzgar_Hizi", anlikRuzgar)) {
    Serial.println("-> Başarılı: Yeni rüzgar hızı satırı eklendi.");
  }
  if (baserow.insertText("Hava_Kalitesi", anlikDurum + "_LOG")) {
    Serial.println("-> Başarılı: Yeni günlük metin satırı eklendi.");
  }

  delay(2000);

  Serial.println("\n==================================================");
  Serial.println("📂 SABİT SATIR GÜNCELLEME (JSON KULLANMADAN)");
  Serial.println("==================================================");
  
  // Baserow panelindeki 1 numaralı satırı günceller
  if (baserow.updateNumber(1, "Ruzgar_Hizi", anlikRuzgar)) {
    Serial.println("-> Başarılı: ID:1 olan satırdaki rüzgar hızı güncellendi.");
  }
  if (baserow.updateText(1, "Hava_Kalitesi", anlikDurum)) {
    Serial.println("-> Başarılı: ID:1 olan satırdaki durum metni güncellendi.");
  }

  delay(2000);

  Serial.println("\n==================================================");
  Serial.println("📂 VERİ OKUMA (JSON KULLANMADAN)");
  Serial.println("==================================================");

  // 🎯 Baserow arayüzünde görünen 1. satırı direkt olarak okuyoruz
  float ledEmri = baserow.readNumber("LED_Komut", 1);
  if (ledEmri == 1.0) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("-> readNumber() Sonucu: LED AÇILDI.");
  } else if (ledEmri == 0.0) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("-> readNumber() Sonucu: LED KAPATILDI.");
  }

  // 🎯 Baserow arayüzünde görünen 1. satırı direkt olarak okuyoruz
  String havaDurumu = baserow.readText("Hava_Kalitesi", 1);
  Serial.print("-> readText() Sonucu (Hava Durumu): ");
  Serial.println(havaDurumu);

  Serial.println("==================================================");
  delay(30000); // 30 saniye bekle
}
```

---

## 🛠️ Teknik Fonksiyon Referansı (API Reference)

### 📤 Veri Yazma ve Güncelleme
- `bool insertNumber(String fieldName, float value);` -> Tablonun en altına yeni bir sayı satırı ekler.
- `bool insertText(String fieldName, String text);` -> Tablonun en altına yeni bir metin satırı ekler.
- `bool updateNumber(int rowId, String fieldName, float value);` -> Belirtilen `rowId` numaralı satırdaki sayı hücresini günceller.
- `bool updateText(int rowId, String fieldName, String text);` -> Belirtilen `rowId` numaralı satırdaki metin hücresini günceller.

### 📥 Veri Okuma
- `float readNumber(String fieldName, int rowNumber = 1);` -> Baserow'da görünen satır numarasına göre sayısal veya boolean veriyi okur (Hata durumunda `-1.0` döner).
- `String readText(String fieldName, int rowNumber = 1);` -> Baserow'da görünen satır numarasına göre metinsel veriyi okur.

---

## 📄 Lisans
Bu proje [MIT](LICENSE) lisansı altında korunmaktadır. Özgürce kullanabilir, dağıtabilir ve çatallayabilirsiniz (fork).

