# BaserowMCU 1.6.0 — ESP32 / ESP8266

ESP32 ve ESP8266 Arduino kartları için hızlandırılmış Baserow istemcisi.
Kaynak: https://github.com/mikrodunya/BaseRow-for-ESP8266-and-ESP32-Arduino

## Kurulum

Arduino IDE > Sketch / Taslak > Include Library / Kütüphane Ekle > Add .ZIP Library
ile BaserowMCU-1.6.0.zip dosyasını ekleyin. Eski BaserowMCU sürümünün yedeğini alın;
IDE aynı adlı kütüphane nedeniyle kurulumu reddederse önce eski klasörü taşıyın.
ArduinoJson 7 veya üzeri gerekir. Araçlar > Kart menüsünden kullandığınız kartı seçin.
Kütüphane bu seçime göre ESP32 / ESP8266 kodunu otomatik derler; ek bir ayar veya
WiFi include değişikliği gerekmez. Bilgisayara takılan kartın modelini kendisi
keşfetmez; Arduino IDE'de doğru kartı seçmek yine gereklidir.

Dosya > Örnekler > BaserowMCU > Baserow_Example örneğini açın. WiFi, table ID,
database token ve gerçek ROW_ID değerlerini girin. Örnek mevcut satırdaki
Ruzgar_Hizi ve Hava_Kalitesi alanlarını değiştirir. Önce test tablosu kullanın.

## Hız değişiklikleri

- HTTPClient ve WiFiClientSecure nesneleri istekler arasında korunur.
- HTTP keep-alive varsayılan açıktır; sunucu izin verdiğinde aynı TCP/TLS bağlantısı kullanılır.
- Yazma yanıtları da tamamen tüketilir; okunmamış JSON sonraki isteğe karışmaz.
- ESP8266'da sunucunun desteklediği durumlarda BearSSL TLS oturumu yeniden kullanılır.
- readRow gerçek ID ile yalnızca bir satır getirir. Dönen belgeden birçok alanı okuyabilirsiniz.
- insertRow / updateRow aynı satırın tüm alanlarını bir POST / PATCH içinde yollar.
- Eski readNumber / readText artık size=1&page=N ile tek satır ister.
- Eski fonksiyonlar için isteğe bağlı önbellek vardır; varsayılan kapalıdır.
- Örnekte uzun delay yerine millis zamanlaması vardır. HTTP çağrıları hâlâ senkrondur.

Beş alanı ayrı ayrı yazmak yerine updateRow kullanmak istek sayısını 5'ten 1'e
indirir. Bu, süre için 5 kat hız garantisi değildir. İlk TLS bağlantısı, WiFi,
sunucu gecikmesi ve satır boyutu süreyi etkiler. Sunucu boşta kalan bağlantıyı
kapatabilir; sonraki istek yeniden bağlanır.

## Yeni API

~~~cpp
#include <BaserowMCU.h>
BaserowMCU baserow("TABLE_ID", "DATABASE_TOKEN");

// setup içinde WiFi bağlantısını başlatın.
// Aşağıdaki işlemleri bağlantı kurulduktan sonra yapın:
JsonDocument fields;
fields["Sicaklik"] = 23.5;
fields["Nem"] = 48;
fields["Durum"] = "NORMAL";
bool ok = baserow.updateRow(42, fields); // Tek PATCH, gerçek ID=42.
// baserow.insertRow(fields); // Tek POST, üç alanlı TEK yeni satır.

JsonDocument row;
if (baserow.readRow(42, row)) { // Tek GET.
  float sicaklik = row["Sicaklik"].as<float>();
  String durum = row["Durum"].as<String>();
}
~~~

readRow çıktısını hata durumunda temizler. insertRow ve updateRow bool döndürür;
POST sonucundaki yeni ID bu sürümde dışarıya verilmez.

## Eski kodlarla kullanım

insertNumber, insertText, updateNumber, updateText, readNumber ve readText korunmuştur.
Her insertNumber / insertText hâlâ AYRI bir satır ekler. Aynı satırdaki alanları
birleştirmek için insertRow kullanın. Eski yazma çağrıları otomatik olarak birleştirilmez.

readNumber("Alan", 1) / readText("Alan", 1), API listesindeki birinci kaydı okur.
updateNumber(1, ...) ve readRow(1, ...), ID'si 1 olan kaydı kullanır. Bu ikisi
aynı olmak zorunda değildir. Web arayüzünde filtreleme veya sıralama yaparsanız,
görünen sıra API'nin varsayılan sırasıyla farklı olabilir. Yeni kodda gerçek ID kullanın.
Eski okumalarda 0 veya negatif sıra, önceki davranış gibi 1 kabul edilir.
Sayısal okumada hata -1.0; metinde HATA / KOLON_BULUNAMADI dönüşleri korunur.
Gerçek veri -1 olabileceğinden lastError() kontrolü yapın veya readRow kullanın.

~~~cpp
baserow.setReadCacheMs(250); // Yalnızca eski konumsal okumalara uygulanır.
float n = baserow.readNumber("Sicaklik", 1);
String s = baserow.readText("Durum", 1); // 250 ms içinde aynı satırsa yerelden.
baserow.clearCache();
baserow.setReadCacheMs(0); // Varsayılan: her okuma güncel veriyi ister.
~~~

Önbellek yalnızca son okunan satırı tutar. Süre son dolumdan itibaren ölçülür;
önbellekten okuma süreyi uzatmaz. Yazma girişiminde, bağlantı kapatılırken ve
çevrimdışı okumalarda temizlenir. Dışarıdan yapılan değişiklikler TTL süresince
gecikmeli görülebilir. readRow her zaman ağdan okur; önbelleği kullanmaz.

## Bağlantı ve tanılama

- platform(): ESP32 veya ESP8266 döndürür.
- setKeepAlive(false): Bağlantıyı her istek sonunda kapatır; karşılaştırma için kullanılabilir.
- close(): Soketi kapatır ve önbelleği temizler. Sonraki çağrı yeniden bağlanır.
- setTimeout(5000): HTTP okuma zaman aşımı, ESP32'de ayrıca bağlantı zaman aşımı.
  DNS/TLS dahil toplam işlem süresinin kesin üst sınırı değildir.
- setMaxResponseSize(16384): Okunan JSON için bayt sınırı; daha büyük satırlar için
  kullanılabilir RAM'e göre artırın. JSON ayrıştırma ayrıca bellek kullanır.
- lastHttpCode(): Son HTTP durum kodu; ulaşım hatasında negatif, istek yapılmadıysa 0.
- lastError(): Başarıda boş metin. HTTP hata gövdeleri RAM tasarrufu için saklanmaz.
- lastRequestMs(): Son ağ işleminin süresi; JSON işleme ve payload hazırlığı dahil değildir.
- lastResponseBytes(): Başarıyla tüketilen son yanıt gövdesinin boyutu.
- lastReadFromCache(): Son konumsal okuma önbellekten mi geldi?

GET bağlantı hatasında bir kez yeni bağlantıyla denenir. POST / PATCH otomatik
tekrarlanmaz; yanıt kaybolsa bile sunucu yazmayı tamamlamış olabilir. Hatalı veya
kısmi yanıt alınırsa bağlantı kapatılır. Bir örneği farklı FreeRTOS görevlerinden
eşzamanlı kullanmayın; çağrıları sıraya koyun veya kilitleyin.

## TLS

Önceki sürümle uyumluluk için varsayılan setInsecure davranışı korunmuştur:
HTTPS şifrelenir, fakat sunucunun kimliği doğrulanmaz. Bu bir hızlandırma değildir.
Doğrulama için cihaz saatini senkronize edip setCACert(ROOT_CA_PEM) çağırın.
PEM kütüphanede saklanır. setInsecure() yeniden doğrulamasız moda geçirir.
Yalnızca https:// taban URL kabul edilir; token için düz HTTP kullanılmaz.

## Doğrulama

Derleme ve davranış testi sonuçları TEST-RESULTS.md dosyasındadır.
Donanımda süre ölçümü için örnek her çağrının HTTP kodunu, milisaniyesini ve yanıt
boyutunu seri porta yazar. Aynı WiFi ve tabloyla ilk çağrıyı ve sonraki çağrıları
ayrı karşılaştırın. Bu pakette canlı Baserow hesabına veya fiziksel karta yazma yapılmamıştır.
