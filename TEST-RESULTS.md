# Doğrulama sonuçları — BaserowMCU 1.6.0

Tarih: 27 Ekim 2026
Temel kaynak commit: f5fafeacf421e1e221f5c4f8e19a5bd08f33dba7

## Gerçek Arduino derlemeleri

ArduinoJson 7.4.3 ile Baserow_Example.ino derlendi:

| Hedef | Arduino çekirdeği | Sonuç | Program | Statik RAM |
| --- | --- | --- | --- | --- |
| esp32:esp32:esp32 | 3.2.0 | Başarılı | 1.033.874 bayt | 48.760 bayt |
| esp8266:esp8266:nodemcuv2 | 2.5.2 | Başarılı | 407.952 bayt | 29.940 bayt |

RAM rakamları derleyicinin statik kullanım raporudur; çalışma anındaki TLS ve JSON
tahsislerini içermez. Daha yeni ESP8266 çekirdekleri ve diğer ESP32 varyantları bu
ortamda ayrıca derlenmedi.

## Çalıştırılan davranış testleri

Gerçek BaserowMCU.cpp ve ArduinoJson 7.4.3, taklit Arduino/WiFi/HTTP katmanıyla
masaüstünde derlenip çalıştırıldı. ESP32 ve ESP8266 koşullu derleme yolları geçti.
Testler extras/tests altında; C++ derleyicisi ve ArduinoJson src klasörü ile
run.py üzerinden tekrar çalıştırılabilir. Bu çalıştırmada Zig 0.13.0 C++ kullanıldı.

- Doğru satır URL'si, token başlığı, tek GET ile çok alan okuma.
- Tek PATCH / POST içinde çok alan ve eski tek-alan yazma fonksiyonları.
- İstekler arasında aynı istemci, yanıtı tüketme ve eski başlıkların temizlenmesi.
- Önbelleğin varsayılan kapalı olması; TTL, süre taşması, sıra anahtarı ve yazma sonrası geçersizleştirme.
- Çevrimdışı durumda önbelleği kullanmama.
- GET ulaşım hatasında bir tekrar; POST/PATCH için tekrar yapmama.
- HTTP hata kodları, eksik yanıt, boyut sınırı, bilinmeyen uzunlukta parça parça gövde.
- Bozuk JSON, yanlış satır ID'si, boş sonuç ve bulunamayan alan.
- Sunucunun bağlantıyı kapatması ve keep-alive kapalı davranış.
- Hata durumunda eski JSON sonucunun temizlenmesi.

## Sınırlar

Fiziksel ESP kartında veya canlı Baserow hesabında işlem yapılmadı. Taklit HTTP
katmanı gerçek TLS el sıkışmasını, kablosuz ağı veya gerçek HTTP chunk çözümlemesini
çalıştırmaz; bunlar kart çekirdeğinin HTTPClient uygulamasına bırakılır. Bu nedenle
milisaniye veya hızlanma yüzdesi iddia edilmemektedir. Örneğin seri port çıktısıyla
kendi ağınızda ilk ve sonraki isteklerin süresini karşılaştırın.
