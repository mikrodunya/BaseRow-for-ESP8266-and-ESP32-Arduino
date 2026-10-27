# BaserowMCU 🚀

**ESP32 ve ESP8266 için Baserow Arduino kütüphanesi — v1.6.0**

![Sürüm](https://img.shields.io/badge/version-1.6.0-blue)
![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20ESP8266-green)
![ArduinoJson](https://img.shields.io/badge/ArduinoJson-7%2B-orange)

BaserowMCU, WiFi bağlantılı mikrodenetleyicilerinizden [Baserow](https://baserow.io) tablolarına veri yazmanızı, mevcut kayıtları güncellemenizi ve cihazınıza komut okumanızı sağlayan bir Arduino kütüphanesidir. Sensör kayıtları, uzaktan LED kontrolü ve IoT durum panelleri için kullanılabilir.

**1.6.0 sürümü**, bağlantının tekrar kullanılması, tek satır okuma ve aynı satırdaki alanların tek istekte gönderilmesiyle gereksiz ağ işlemlerini azaltır. Önceki sürümün tek alanla çalışan fonksiyonları da kullanılmaya devam eder.

[GitHub deposu](https://github.com/mikrodunya/BaseRow-for-ESP8266-and-ESP32-Arduino) · [Mikrodünya](https://mikrodunya.wordpress.com) · [Baserow API belgeleri](https://baserow.io/user-docs/database-api)

---

## 📑 İçindekiler

- [Özellikler](#ozellikler)
- [Donanım ve bağımlılıklar](#donanim)
- [Kurulum](#kurulum)
- [Baserow hazırlığı](#baserow-hazirligi)
- [Satır ID’si ve liste sırası](#satir-id)
- [Hızlı başlangıç](#hizli-baslangic)
- [Veri ekleme, güncelleme ve okuma](#veri-islemleri)
- [Fonksiyon referansı](#api)
- [Önbellek ve bağlantı ayarları](#ayarlar)
- [Haberleşme hızını ölçme](#hiz-olcumu)
- [HTTPS ve sertifika doğrulama](#https)
- [Sorun giderme](#sorun-giderme)
- [1.5.0’dan geçiş ve doğrulama](#gecis)

---

<a id="ozellikler"></a>
## ✨ Özellikler

- **🔍 Otomatik platform seçimi:** Arduino IDE’de seçilen karta göre ESP32 veya ESP8266 başlıkları yüklenir. Kodda `WiFi.h` / `ESP8266WiFi.h` değişikliği gerekmez.
- **🔄 Çift yönlü iletişim:** `POST` ile yeni satır oluşturma, `PATCH` ile güncelleme ve `GET` ile okuma.
- **📦 Birden çok alan, tek istek:** `insertRow()` ve `updateRow()` ile bir satırın alanlarını birlikte gönderme.
- **🎯 ID ile doğrudan okuma:** `readRow()` ile yalnızca istenen satırı alma; aynı yanıttan birden çok alan okuma.
- **⚡ Bağlantı tekrar kullanımı:** Sunucu desteklediğinde aynı TCP/TLS bağlantısıyla sonraki istekleri gönderme.
- **🧠 İsteğe bağlı önbellek:** Eski `readNumber()` / `readText()` çağrılarında aynı satırı kısa süreyle tekrar kullanma.
- **📊 Süre ve hata takibi:** HTTP durum kodu, hata açıklaması, işlem süresi ve yanıt boyutuna erişim.
- **🧩 Eski fonksiyonlarla kullanım:** Tek bir sayı veya metin için JSON belgesi oluşturmadan işlem yapma.

> **Otomatik seçim nasıl çalışır?** IDE’de doğru kartı seçmeniz gerekir. Kütüphane derleme hedefini otomatik tanır; USB’ye takılan kartın modelini kendisi keşfetmez.

<a id="donanim"></a>
## 🔌 Donanım ve bağımlılıklar

| Platform | Örnek kartlar | Açıklama |
| --- | --- | --- |
| ESP32 | ESP32 Dev Module, ESP32-C3, WiFi destekli ESP32-S2/S3 kartları | ESP32 Arduino çekirdeği kullanılır. |
| ESP8266 | NodeMCU, WeMos D1 Mini | ESP8266 Arduino çekirdeği ve BearSSL kullanılır. |
| Diğer mimariler | Klasik AVR Uno, Nano, Mega | Bu kütüphane tarafından desteklenmez. |

Gereken yazılımlar:

| Bileşen | Gereklilik |
| --- | --- |
| Arduino IDE veya Arduino CLI | Derleme ve yükleme için |
| ESP32 / ESP8266 kart paketi | Kullanılacak karta uygun çekirdek |
| **ArduinoJson 7 veya üzeri** | JSON oluşturma ve ayrıştırma için |
| Adafruit NeoPixel | Yalnızca RGB LED kullanan `Baserow_Hiz_Testi` örneği için |

WiFi, HTTPS ve HTTPClient bileşenleri kart paketleriyle gelir. Tüm kart varyantları fiziksel olarak test edilmiş değildir; derlenmiş hedefler [doğrulama bölümünde](#gecis) belirtilmiştir.

<a id="kurulum"></a>
## 📥 Kurulum

1. Arduino IDE’nin Kart Yöneticisi üzerinden kartınıza uygun ESP32 veya ESP8266 paketini kurun.
2. Kütüphane Yöneticisi’nde **ArduinoJson**, yazar **Benoit Blanchon** araması yapıp 7.x veya üzerini kurun.
3. Hazır kütüphane ZIP’iniz varsa **Taslak → Kütüphane Ekle → ZIP Kitaplığı Ekle** yolunu kullanın.
4. Depoyu ZIP olarak indirdiyseniz arşivi açın. İçinde `library.properties` bulunan **BaserowMCU** klasörünü Arduino sketchbook dizininizdeki `libraries` klasörüne yerleştirin.
5. **Araçlar → Kart** menüsünden kartınızı seçin; örnekleri **Dosya → Örnekler → BaserowMCU** altında açın.

Beklenen kurulum yapısı:

```text
<Arduino sketchbook>/libraries/BaserowMCU/
├── library.properties
├── BaserowMCU.h
├── BaserowMCU.cpp
└── examples/
    ├── Baserow_Example/
    └── Baserow_Hiz_Testi/
```

Eski sürüm kuruluysa yedeğini alıp bu klasörü güncelleyin. Derleyici çıktısındaki **Used / Kullanılan** kütüphane yolunu kontrol edin: masaüstündeki dosyayı değiştirmek, Arduino’nun başka konumdaki kurulumunu otomatik güncellemez.

<a id="baserow-hazirligi"></a>
## 🔑 Baserow hazırlığı

### 1. Database token oluşturun

Baserow çalışma alanının **Settings → Database tokens** bölümünden bir token oluşturun. Kullanacağınız tablo için okuma, yeni kayıt ekleme ve güncelleme işlemlerine uygun **Read / Create / Update** izinlerini verin. Kütüphane, token’ı `Authorization: Token ...` başlığıyla gönderir. Ayrıntılar: [Baserow database token rehberi](https://baserow.io/user-docs/personal-api-tokens).

### 2. Table ID’yi bulun

Tablonun adresindeki `/table/` sonrasında yer alan değer tablo ID’sidir. Örneğin `.../database/12/table/3456` adresinde tablo ID’si `3456` olur. Veritabanı ID’si ile tablo ID’sini karıştırmayın. [ID bulma rehberi](https://baserow.io/user-docs/database-and-table-id)

### 3. Alanları ve örnek satırı hazırlayın

Örneklerde kullanılan alanlar:

| Alan adı | Baserow türü | Kullanım |
| --- | --- | --- |
| `Sicaklik` | Number | Cihazın gönderdiği örnek sıcaklık |
| `Durum` | Text | Metin hücresi ve satır işlemleri için |
| `Led_Komut` | Number | Yalnızca LED hız testi örneğinde kullanılır: `0` veya `1` |

Bir satır oluşturun ve **gerçek satır ID’sini** öğrenin. API’nin döndürdüğü satır nesnesindeki `id` bu değerdir. Şemanıza özel uç noktaları veritabanının **View API Docs** bölümünden görebilirsiniz. [Baserow API rehberi](https://baserow.io/user-docs/database-api)

<a id="satir-id"></a>
## 🎯 Satır ID’si ile liste sırası farklıdır

| Çağrı | Sayının anlamı |
| --- | --- |
| `readRow(42, row)` | Gerçek ID’si **42** olan satır |
| `updateNumber(42, "Sicaklik", 25)` | Gerçek ID’si **42** olan satır |
| `updateRow(42, fields)` | Gerçek ID’si **42** olan satır |
| `readNumber("Sicaklik", 1)` | API listesindeki **birinci** satır |
| `readText("Durum", 1)` | API listesindeki **birinci** satır |

Örneğin API listesinin ilk kaydının ID’si `42` olabilir. Bu durumda `readNumber("Sicaklik", 1)` ile `readRow(42, row)` aynı kaydı okur; `readRow(1, row)` farklı bir kayıt ister.

Web arayüzündeki filtre ve sıralamalar API’nin varsayılan sırasıyla aynı olmak zorunda değildir. **Yeni projelerde `readRow()` ve gerçek satır ID’si kullanılması önerilir.**

<a id="hizli-baslangic"></a>
## 🚀 Hızlı başlangıç

Tablonuzda **Sicaklik** adlı bir Number alanı ve **Durum** adlı bir Text alanı oluşturun. WiFi bilgilerini, tablo ID’sini ve token’ı aşağıdaki koda yazın. `SATIR_ID` değerini tablonuzda zaten bulunan bir satırın gerçek ID’siyle değiştirin.

Örnek dokuz fonksiyonun her birini bir kez kullanır. İşlemler `setup()` içinde olduğundan cihaz her başlatıldığında bir kez çalışır. Üç `insert` çağrısı toplam **üç ayrı yeni satır** oluşturur.

<a id="veri-islemleri"></a>
## 📤 Veri ekleme, güncelleme ve okuma

```cpp
#include <BaserowMCU.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
BaserowMCU baserow("YOUR_TABLE_ID", "YOUR_DATABASE_TOKEN");

const int SATIR_ID = 1;   // Guncellenecek mevcut satirin gercek ID'si.
const int SATIR_SIRA = 1; // API listesindeki birinci satir.

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi baglandi.");

  // 1. Yeni satira bir sayi yaz.
  Serial.print("insertNumber: ");
  Serial.println(baserow.insertNumber("Sicaklik", 25.5));

  // 2. Baska bir yeni satira metin yaz.
  Serial.print("insertText: ");
  Serial.println(baserow.insertText("Durum", "NORMAL"));

  // 3. Mevcut satirin sayi hucresini guncelle.
  Serial.print("updateNumber: ");
  Serial.println(baserow.updateNumber(SATIR_ID, "Sicaklik", 26.0));

  // 4. Mevcut satirin metin hucresini guncelle.
  Serial.print("updateText: ");
  Serial.println(baserow.updateText(SATIR_ID, "Durum", "CALISIYOR"));

  // 5. Iki alani birlikte, tek bir yeni satira yaz.
  JsonDocument veriler;
  veriler["Sicaklik"] = 27.5;
  veriler["Durum"] = "YENI KAYIT";
  Serial.print("insertRow: ");
  Serial.println(baserow.insertRow(veriler));

  // 6. Mevcut satirin iki alanini tek istekte guncelle.
  veriler["Sicaklik"] = 28.0;
  veriler["Durum"] = "GUNCELLENDI";
  Serial.print("updateRow: ");
  Serial.println(baserow.updateRow(SATIR_ID, veriler));

  // 7. Gercek ID ile tum satiri oku ve ekrana yazdir.
  JsonDocument satir;
  if (baserow.readRow(SATIR_ID, satir)) {
    Serial.println("Tum satir:");
    serializeJsonPretty(satir, Serial);
    Serial.println();
  } else {
    Serial.println(baserow.lastError());
  }

  // 8. Listedeki birinci satirin sayi hucresini oku.
  float sicaklik = baserow.readNumber("Sicaklik", SATIR_SIRA);
  Serial.print("Sicaklik: ");
  if (baserow.lastError().length() == 0) Serial.println(sicaklik);
  else Serial.println(baserow.lastError());

  // 9. Listedeki birinci satirin metin hucresini oku.
  String durum = baserow.readText("Durum", SATIR_SIRA);
  Serial.print("Durum: ");
  if (baserow.lastError().length() == 0) Serial.println(durum);
  else Serial.println(baserow.lastError());
}

void loop() {
  // Ornek islemler setup() icinde bir kez yapilir.
}
```

Seri Monitör’ü **115200 baud** ile açın. Yazma işlemlerinin yanında görünen `1` başarılı, `0` başarısız anlamına gelir. Hata açıklamasını almak için ilgili çağrıdan hemen sonra `baserow.lastError()` okuyabilirsiniz.

**`SATIR_ID` ve `SATIR_SIRA` farklı kavramlardır:** `readRow()` ve güncelleme fonksiyonları gerçek ID kullanır. `readNumber()` ve `readText()` ise API listesindeki sırayı kullanır. İki değişkene de `1` yazılması, aynı kaydı gösterecekleri anlamına gelmez.

`insertNumber()` ve `insertText()` aynı satırın iki hücresini doldurmaz; ayrı satırlar ekler. Alanları aynı yeni satıra yazmak için `insertRow()` kullanın. `updateRow()` yalnızca `veriler` belgesine eklediğiniz alanları günceller; diğer alanları değiştirmez. `readRow()` ise satırın dönen tüm alanlarını belgeye alır.

### Veri türleri

| Arduino / JSON değeri | Baserow alanı | Gönderme yöntemi |
| --- | --- | --- |
| Sayı: `24.5`, `100` | Number | `insertNumber()` / `updateNumber()` veya `JsonDocument` |
| Metin: `"NORMAL"` | Text | `insertText()` / `updateText()` veya `JsonDocument` |
| Boolean: `true`, `false` | Boolean / Checkbox | `JsonDocument` içinde gerçek boolean değer |
| Dizi veya nesne | Alanın beklediği API biçimi | `JsonDocument` ile; tablonun API şemasına uygun yapı |

Örneğin bir Checkbox alanı için `fields["Aktif"] = true;` kullanın. Boolean alanlara sayısal `0/1` gönderilmesini varsaymayın. Tek alanlı sayısal fonksiyonların parametresi `float` olduğundan, büyük tam sayılar için uygun türde `JsonDocument` değeri tercih edin.

<a id="api"></a>
## 🛠️ Fonksiyon referansı

### Bağlantı nesnesi

```cpp
BaserowMCU baserow("TABLE_ID", "DATABASE_TOKEN");

// Kendi Baserow sunucunuz icin:
// BaserowMCU baserow("TABLE_ID", "DATABASE_TOKEN", "https://baserow.example.com");
```

Varsayılan sunucu `https://api.baserow.io` adresidir. Yalnızca `https://` taban URL kabul edilir. WiFi bağlantısını kullanıcı kodu başlatır.

### Yazma ve okuma

| Fonksiyon | Dönüş | Açıklama |
| --- | --- | --- |
| `insertNumber(String fieldName, float value)` | `bool` | Tek sayısal alanlı yeni satır ekler. |
| `insertText(String fieldName, String text)` | `bool` | Tek metin alanlı yeni satır ekler. |
| `updateNumber(int rowId, String fieldName, float value)` | `bool` | Gerçek ID ile sayısal alanı günceller. |
| `updateText(int rowId, String fieldName, String text)` | `bool` | Gerçek ID ile metin alanını günceller. |
| `insertRow(const JsonDocument& fields)` | `bool` | Birden çok alanı tek yeni satıra ekler. |
| `updateRow(int rowId, const JsonDocument& fields)` | `bool` | Gerçek ID ile alanları birlikte günceller. |
| `readRow(int rowId, JsonDocument& result)` | `bool` | Gerçek ID ile bir satırı okur. |
| `readNumber(String fieldName, int rowNumber = 1)` | `float` | API listesindeki sıra ile okur; hatada `-1.0`. |
| `readText(String fieldName, int rowNumber = 1)` | `String` | Sıra ile okur; hatada `HATA`, alan yoksa/boşsa `KOLON_BULUNAMADI`. |

`bool` sonuçlar başarıda `true`, hatada `false` olur. `readNumber()` sonucundaki `-1` gerçek veri de olabileceğinden `lastError()` ile kontrol edin. ID kullanan fonksiyonlarda pozitif bir ID gerekir. Eski konumsal okumalarda `0` veya negatif sıra `1` kabul edilir. Satır silme fonksiyonu bu sürümde bulunmaz.

### Ayarlar

| Fonksiyon | Açıklama / varsayılan |
| --- | --- |
| `setTimeout(uint16_t timeoutMs)` | HTTP zaman aşımı; varsayılan **5000 ms**. ESP32’de bağlantı zaman aşımına da uygulanır. |
| `setKeepAlive(bool enabled)` | Bağlantı tekrar kullanımını ayarlar; varsayılan **true**. Ayar değişiminde mevcut bağlantı kapanır. |
| `setReadCacheMs(uint32_t ttlMs)` | Eski konumsal okuma önbelleğinin süresi; varsayılan **0**, kapalı. |
| `clearCache()` | Okuma önbelleğini temizler. |
| `close()` | Bağlantıyı kapatır ve önbelleği temizler; sonraki çağrı yeniden bağlanabilir. |
| `setMaxResponseSize(size_t bytes)` | Okunan JSON gövdesinin sınırı; varsayılan **16384 bayt**. |
| `setCACert(const char* pem)` | Sunucu doğrulaması için PEM biçiminde CA sertifikası ayarlar; metin içeride saklanır. |
| `setInsecure()` | TLS sunucu kimliği doğrulamasını kapatır; uyumluluk için varsayılan davranış budur. |

### Tanılama ve performans

| Fonksiyon | Dönüş | Açıklama |
| --- | --- | --- |
| `BaserowMCU::platform()` | `const char*` | Derleme hedefine göre `ESP32` veya `ESP8266`. |
| `lastHttpCode()` | `int` | Son HTTP kodu; ulaşım hatasında negatif, istek yapılmadıysa `0`. |
| `lastError()` | `const String&` | Son işlemin hata açıklaması; başarıda boş. |
| `lastRequestMs()` | `uint32_t` | Son ağ işleminin süresi; JSON hazırlama ve ayrıştırma hariç. |
| `lastResponseBytes()` | `size_t` | Son başarıyla tüketilen yanıt gövdesinin bayt sayısı. |
| `lastReadFromCache()` | `bool` | Son konumsal okumanın önbellekten gelip gelmediği. |

Tanılama bilgilerini ilgili çağrıdan hemen sonra okuyun; sonraki veri işlemi bunları değiştirir. HTTP `200` alınmış olsa bile JSON ayrıştırma başarısız olabilir; asıl başarı kontrolü fonksiyonun dönüş değeriyle yapılmalıdır.

<a id="ayarlar"></a>
## ⚙️ Önbellek ve bağlantı ayarları

### Kısa süreli okuma önbelleği

```cpp
baserow.setReadCacheMs(250);
float sicaklik = baserow.readNumber("Sicaklik", 1);
String durum = baserow.readText("Durum", 1);
// Ikinci cagri 250 ms icinde ayni satira yapilirsa onbellekten gelebilir.

Serial.println(baserow.lastReadFromCache());
baserow.clearCache();
baserow.setReadCacheMs(0); // Her okumada agdan veri istemeye don.
```

Önbellek yalnızca son okunan satırı tutar; süre son dolumdan itibaren ölçülür. Sunucuya yazma girişiminde ve bağlantı kapatıldığında temizlenir. Çevrimdışı konumsal okumada eski veri döndürülmez. Dışarıdan yapılan değişiklikler önbellek süresince gecikmeli görülebilir. **`readRow()` bu önbelleği kullanmaz; her çağrıda ağdan okur.**

### Bağlantı ömrü ve hata davranışı

Sunucu bağlantıyı açık tutmayı kabul ederse istekler aynı soket üzerinden devam eder. Sunucu bağlantıyı kapatabilir; `setKeepAlive(true)` bağlantının sürekli açık kalacağını garanti etmez. ESP8266’da, sunucu desteklediğinde TLS oturumu da yeniden kullanılabilir.

GET isteği yanıt başlığı alınmadan oluşan ulaşım hatasında bir kez yeniden denenebilir. POST/PATCH otomatik tekrarlanmaz: yanıt kaybolmuş olsa bile sunucu yazmayı tamamlamış olabilir. Eksik gövde alınırsa bağlantı kapatılır ve hata döndürülür.

`setTimeout()` DNS ve TLS dahil tüm işlem süresinin kesin üst sınırı değildir. Tek bir `BaserowMCU` nesnesine birden fazla görevden eşzamanlı erişmeyin; çağrıları sıraya koyun veya kilitleyin.

<a id="hiz-olcumu"></a>
## ⏱️ Haberleşme hızını ölçme

Bir API çağrısının toplam süresini ölçmek için:

```cpp
uint32_t baslangic = millis();
bool ok = baserow.updateNumber(42, "Sicaklik", 25.5);
uint32_t apiMs = millis() - baslangic; // Serial ciktilarindan once olculur.

Serial.printf("Basari=%d | API=%lu ms | HTTP=%lu ms\n", ok,
    static_cast<unsigned long>(apiMs),
    static_cast<unsigned long>(baserow.lastRequestMs()));
```

**API süresi**, çağrı içindeki JSON işlemleriyle birlikte ölçülür. **HTTP süresi** bunun içindedir; iki değeri toplamayın. İlk istekte yeni TLS bağlantısı kurulduğundan ilk turu sonraki turlardan ayrı değerlendirin.

Hazır [Baserow_Hiz_Testi](BaserowMCU/examples/Baserow_Hiz_Testi/Baserow_Hiz_Testi.ino) örneği ESP32-C3 NeoPixel kontrolünü sürdürürken yazma, okuma, bekleme ve tur sürelerini ayrı gösterir. İlk tur hariç başarılı turların ortalama, minimum ve maksimum API toplamını hesaplar. Ayrıntılı kullanım: [hız testi kılavuzu](BaserowMCU/examples/Baserow_Hiz_Testi/README.md).

| Deneme | `TOPLU_YAZMA` | `KALICI_BAGLANTI` | Tur başına istek |
| --- | --- | --- | --- |
| İki ayrı yazma, her istekte yeni bağlantı | `false` | `false` | 3 |
| İki ayrı yazma, bağlantı tekrar kullanımı | `false` | `true` | 3 |
| İki alan tek yazma, bağlantı tekrar kullanımı | `true` | `true` | 2 |

Her ayar değişikliğinden sonra yeniden yükleyin; aynı ağ ve tabloyla en az 10 tur karşılaştırın. Bu üç deneme de 1.6.0 kütüphanesiyle çalışır; 1.5.0 sürümünün birebir testi değildir. Örnek için `secrets.h` dosyasındaki ayarları doldurun ve Seri Monitör’ü **115200 baud** ile açın.

İki alanı birleştirmek yazma isteği sayısını 2’den 1’e indirir; süreyi mutlaka yarıya indirmez. Gerçek sonuç WiFi, sunucu, veri boyutu ve bağlantının yeniden kurulmasına bağlıdır. **Ölçülmüş bir hızlanma yüzdesi henüz yayımlanmamıştır.** API süresi, mobil uygulamadan LED’e kadar uçtan uca komut gecikmesi değildir.

<a id="https"></a>
## 🔒 HTTPS ve sertifika doğrulama

Önceki sürümle uyumluluk için varsayılan TLS davranışı `setInsecure()` şeklindedir. Bu modda trafik şifrelenir ancak **sunucunun kimliği doğrulanmaz**.

Doğrulama için cihaz saatini senkronize edin ve bağlandığınız sunucuya uygun kök CA sertifikasını `baserow.setCACert(ROOT_CA_PEM);` ile ayarlayın. Bu çağrı mevcut bağlantıyı kapatır; sonraki bağlantı yeni ayarla kurulur.

README’deki kimlik bilgileri yer tutucudur. Kendi ayarlarınızı ayrı bir `secrets.h` dosyasında tutabilir, bu dosyayı `.gitignore` kapsamına alabilirsiniz. Git tarafından önceden izlenen bir dosyayı `.gitignore` içine eklemek, geçmişteki içeriğini kaldırmaz.

<a id="sorun-giderme"></a>
## 🧰 Sorun giderme

| Belirti | Kontrol edilecek nokta |
| --- | --- |
| `updateRow` / `readRow` bulunamıyor | Arduino IDE eski BaserowMCU kopyasını kullanıyor olabilir; kullanılan yolu ve `library.properties` sürümünü kontrol edin. |
| ArduinoJson sürüm hatası | ArduinoJson 7 veya üzerini kurun. |
| `WiFi is not connected` | SSID, şifre ve `WiFi.status()` değerini kontrol edin. Kütüphane WiFi bağlantısını kendisi başlatmaz. |
| HTTP `400` | Alan adları, alan türleri ve gönderilen veri biçimini kontrol edin. |
| HTTP `401` / `403` | Database token ve tablo izinlerini kontrol edin. |
| HTTP `404` | Sunucu adresi, Table ID ve gerçek Row ID’yi kontrol edin. |
| `Field not found or null` | Alan adını ve alanın boş olup olmadığını kontrol edin. |
| `Response exceeds max response size` | Satır büyükse RAM durumuna göre `setMaxResponseSize()` sınırını artırın. |
| `Incomplete HTTP response` | WiFi/sunucu bağlantısını ve zaman aşımını kontrol edin. |
| HTTP başarılı, fonksiyon başarısız | `lastError()` değerini okuyun; JSON veya beklenen satır yapısı hatalı olabilir. |
| İlk çağrı yavaş | İlk bağlantının TLS kurulumu içerdiğini dikkate alın; sonraki çağrıları da ölçün. |
| LED komutu geç uygulanıyor | API süresinin yanında sorgulama aralığını ve `delay()` sürelerini kontrol edin. |

Yanıt boyutu sınırı toplam RAM tüketimi değildir; TLS tamponları ve ayrıştırılmış JSON ayrıca bellek kullanır. Özellikle ESP8266’da büyük satırları okurken boş belleği takip edin.

<a id="gecis"></a>
## 📝 1.5.0’dan geçiş ve doğrulama

**Korunan fonksiyonlar:** `insertNumber`, `insertText`, `updateNumber`, `updateText`, `readNumber` ve `readText`.

**Yeni veya değişen davranışlar:**

- `insertRow`, `updateRow` ve `readRow` ile satır bazında işlem yapılır.
- Eski konumsal okumalar `size=1&page=N` ile yalnızca istenen sıradaki satırı ister.
- Bağlantı tekrar kullanımı varsayılan açık, okuma önbelleği varsayılan kapalıdır.
- Birden çok eski yazma çağrısı kendiliğinden birleştirilmez; bunun için `updateRow` / `insertRow` kullanın.
- Özel sunucu adresi `https://` ile başlamalıdır.
- `BaserowMCU` nesnesi kopyalanamaz; fonksiyonlara referansla aktarın.

ArduinoJson **7.4.3** ile doğrulanan derlemeler:

| Hedef | Çekirdek | Örnek | Sonuç |
| --- | --- | --- | --- |
| ESP32 Dev Module | ESP32 3.2.0 | `Baserow_Example` | ✅ Derlendi |
| NodeMCU ESP8266 | ESP8266 2.5.2 | `Baserow_Example` | ✅ Derlendi |
| ESP32-C3 | ESP32 3.2.0 | `Baserow_Hiz_Testi` | ✅ Derlendi |

Bağlantı kullanımı, önbellek, toplu yazma ve hata durumları masaüstünde taklit ağ katmanıyla test edilmiştir. Bu testler gerçek WiFi/TLS gecikmesinin veya fiziksel donanım testinin yerini tutmaz. Test kapsamı: [TEST-RESULTS.md](BaserowMCU/TEST-RESULTS.md).

---

## 📄 Lisans

[Özgün projenin README’sinde](https://github.com/mikrodunya/BaseRow-for-ESP8266-and-ESP32-Arduino#readme) belirtilen lisans **MIT**’dir.

**Proje:** [Mikrodünya — BaserowMCU](https://github.com/mikrodunya/BaseRow-for-ESP8266-and-ESP32-Arduino)
