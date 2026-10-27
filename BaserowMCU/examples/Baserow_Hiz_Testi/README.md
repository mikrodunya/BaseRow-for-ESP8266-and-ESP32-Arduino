# ESP32-C3 Baserow haberleşme süresi ölçümü

Yerel çalışma dosyası:
C:/Users/Meteor/Desktop/BaserowMCU/examples/Baserow_Hiz_Testi/Baserow_Hiz_Testi.ino

Kütüphane: BaserowMCU 1.6.0. Diğer bağımlılık: Adafruit NeoPixel.
Yerel secrets.h içinde gönderdiğiniz kodun Baserow ayarları korunmuştur.
WiFi adı ve şifresi gönderilen koddaki yer tutuculardır; bunları doldurun.
İndirilebilir kopyadaki secrets.h tamamen yer tutucu içerir.

## Çalıştırma

1. BaserowMCU 1.6.0 sürümünün Arduino IDE tarafından kullanıldığından emin olun.
   Masaüstündeki klasörü değiştirmek, Documents/Arduino/libraries altındaki eski
   kurulumu otomatik güncellemez. Eski kurulumun yedeğini alarak 1.6.0 ile değiştirin.
2. Baserow_Hiz_Testi.ino dosyasını açın; secrets.h içindeki bağlantı ayarlarını doldurun.
3. ESP32C3 Dev Module (veya kullandığınız C3 kartının doğru profilini) ve portunu seçin.
4. Yükleyin ve Seri Monitör'ü 115200 baud ile açın. Dahili USB CDC kullanıyorsanız
   kartın USB CDC On Boot ayarının seri çıktı için uygun olması gerekir.
5. En az 10 tur çıktısını alın. İlk istek bağlantı kurulumunu da içerir.

GPIO 8 ve NeoPixel renk düzeni gönderdiğiniz koddaki gibi korunmuştur. Yazılan
alanlar Sicaklik ve Led_Durum, okunan alan Led_Komut'tur. Led_Durum önceki komutun
sonucunu bir sonraki turda bildirir; gönderdiğiniz akıştaki davranış korunmuştur.
Okuma artık gerçek ID=1 satırını kullanır. Önceki readNumber(..., 1) API listesindeki
ilk kaydı okuyordu. ROW_ID'yi kontrol etmek istediğiniz gerçek satıra ayarlayın.
Ölçüm kodu çalışınca bu satırı orijinal örnekteki gibi günceller.

## Çıktıyı okuma

- ISTEK / API: Çağrı başlamadan hemen önce ve döndükten hemen sonra ölçülür;
  ağ ve fonksiyon içindeki JSON işleme dahildir. Seri port yazdırması dahil değildir.
- ISTEK / HTTP: Kütüphanenin ağ işlem süresi. API süresinin içindedir; ikisini toplamayın.
- Kod: HTTP durum kodu. Hatalar ayrıca açıklanır.
- Yanıt: Sunucudan tüketilen yanıt gövdesi boyutu.
- TUR / API toplam: Bu turdaki yazma ve okuma API sürelerinin toplamı.
- Bekleme: İki delay çağrısında geçen süre; yaklaşık 2000 ms.
- Tur: İşlemler, beklemeler ve tur içindeki seri çıktı/LED işlemleri dahil geçen süre.
  Başlangıç başlığı ve en sondaki özet satırlarının yazdırılması dahil değildir.
- ÖZET: İlk tur hariç, tüm API çağrıları başarılı olan turların API toplamı için
  ortalama/minimum/maksimum. Hatalı turlar ortalamaya katılmaz ve ayrıca sayılır.
- RSSI: Aynı koşullarda karşılaştırma yapabilmek için WiFi sinyal seviyesi.

## Karşılaştırma

Her ayar değişikliğinden sonra yeniden derleyip yükleyin; istatistikler sıfırlanır.
Aynı WiFi, aynı satır, aynı beklemeler ve benzer sayıda örnek kullanın.

| Deneme | TOPLU_YAZMA | KALICI_BAGLANTI | Tur başına istek |
| --- | --- | --- | --- |
| Ayrı istekler, yeni bağlantılar | false | false | 3 |
| Ayrı istekler, kalıcı bağlantı | false | true | 3 |
| Toplu yazma, kalıcı bağlantı (varsayılan) | true | true | 2 |

Birinci ile ikinci bağlantı tekrar kullanımını, ikinci ile üçüncü alanları birleştirmeyi
karşılaştırır. Bu denemelerin hepsi aynı 1.6.0 kütüphanesini kullanır; eski 1.5.0
sürümünün birebir testi değildir. readRow her denemede tek satır okur. Önbellek kapalıdır.
Sunucu bağlantıyı kapatırsa kalıcı bağlantı açıkken de yeniden TLS kurulabilir.

Hızlanma yüzdesi = 100 * (önceki API ortalaması - sonraki API ortalaması) / önceki API ortalaması.
Bu ölçüm mobil uygulamadan LED'e kadar uçtan uca komut gecikmesi değildir. Komutun
sorgulanmayı beklemesi ayrıca gecikme yaratır. İki adet 1 saniyelik bekleme özellikle
korunmuştur; API sürelerinden ayrı raporlanır.

Fiziksel karta yükleme veya canlı Baserow çağrısı bu hazırlık sırasında yapılmadı.
