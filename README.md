🚗 TOPRAK – Akıllı Araba Asistanı
https://img.shields.io/badge/TOPRAK-Ak%C4%B1ll%C4%B1%2520Araba%2520Asistan%C4%B1-blue https://img.shields.io/badge/Version-2.0-green
https://img.shields.io/badge/License-MIT-yellow
https://img.shields.io/badge/Platform-ESP32-red
https://img.shields.io/badge/Arduino-Compatible-orange
ESP32 Tabanlı Gelişmiş Araç İçi Sensör ve Uyarı Sistemi

📌 Proje Özeti

TOPRAK – Akıllı Araba Asistanı, ESP32 mikrodenetleyicisi kullanılarak geliştirilmiş;
mesafe, hava kalitesi, sıcaklık ve nem ölçümü yapan,
OLED ekranlı, joystick kontrollü, sesli ve görsel uyarı sistemine sahip entegre bir araç içi yardımcı sistemdir.

Sistem, gerçek zamanlı sensör verilerini işler, kullanıcı tarafından ayarlanabilir eşik değerlerine göre uyarılar üretir ve sezgisel bir menü arayüzü sunar.

Bu proje:

Gömülü sistemler

IoT

Araç içi elektronik

HMI (Human–Machine Interface)

alanlarında eğitim, prototipleme ve Ar-Ge amaçlı geliştirilmiştir.

🎯 Temel Özellikler

📏 Mesafe Ölçümü (HC-SR04)

Anlık mesafe ölçümü (cm)

Dikkat & tehlike eşikleri

Grafik bar gösterimi

🌫 Hava Kalitesi Ölçümü (MQ-135)

ppm cinsinden gaz seviyesi

Kalite sınıflandırması (İyi / Orta / Kötü)

Ayarlanabilir uyarı eşiği

🌡 Sıcaklık & Nem (DHT11)

°C ve %RH ölçümü

Aşırı sıcaklık uyarısı

🧠 Akıllı Uyarı Sistemi

Çok seviyeli uyarılar

Sesli (buzzer) + görsel (LED) bildirim

Sessiz mod desteği

🕹 Joystick Kontrollü Menü

Yukarı / aşağı: Menü gezinme

Sağ / sol: Değer değiştirme

Basma: Seç / Aç / Kapat

🤖 Animasyonlu Robot Karakter

Göz kırpma animasyonu

Uyarıya göre yüz ifadesi

Sistem durumu görselleştirme

🖥 OLED Grafik Arayüz

128x64 SSD1306

Çoklu ekranlar

Bar grafikler ve ikonlar

🧩 Kullanılan Donanımlar
Donanım	Açıklama
ESP32	Ana kontrolcü
HC-SR04	Ultrasonik mesafe sensörü
MQ-135	Hava kalitesi / gaz sensörü
DHT11	Sıcaklık & nem sensörü
SSD1306 OLED	128x64 I2C ekran
Joystick Modül	Menü kontrolü
Buzzer	Sesli uyarı
LED	Sistem / uyarı göstergesi
🔌 Pin Bağlantıları
Bileşen	ESP32 Pin
OLED SDA	GPIO 21
OLED SCL	GPIO 22
Joystick X	GPIO 34
Joystick Y	GPIO 35
Joystick Button	GPIO 32
Geri Butonu	GPIO 18
HC-SR04 TRIG	GPIO 13
HC-SR04 ECHO	GPIO 12
MQ-135	GPIO 36 (ADC)
DHT11	GPIO 14
Buzzer	GPIO 15
Sistem LED	GPIO 2

⚠️ Not: ESP32 ADC pinleri yalnızca analog giriş içindir. MQ-135 bu nedenle GPIO 36’ya bağlanmıştır.
Kaynak: Espressif ESP32 Datasheet

🕹 Kontroller
Kontrol	İşlev
D18 Butonu	Her zaman ana menü
Joystick Yukarı/Aşağı	Menü gez
Joystick Sağ/Sol	Ayar değiştir
Joystick Basma	Seç / Aç / Kapat
⚙️ Menü Yapısı

Ana Menü

Mesafe Göster

Gaz Göster

Sıcaklık Göster

Mesafe Ayarları

Gaz Ayarları

Sıcaklık Ayarları

Ses Ayarı

Her ayar:

Gerçek zamanlı değiştirilebilir

EEPROM kullanılmadan RAM üzerinde çalışır

Anında uyarı sistemine etki eder

🔊 Uyarı Mantığı
Seviye	Durum	Tepki
0	Normal	Sessiz
1	Dikkat	Yavaş bip
2	Tehlike	Hızlı bip + LED

Ses sistemi tamamen kapatılabilir.

📟 Seri Port Çıktıları

Sistem her 1 saniyede seri porta şu formatta veri yazar:

M:45cm G:620ppm S:28C


Bu çıktı:

Debug

Loglama

Harici sistem entegrasyonu

için kullanılabilir.

🧪 Test Edilen Ortam

Arduino IDE 2.x

ESP32 Core v2.x

Adafruit SSD1306 Library

Adafruit GFX Library

DHT Sensor Library

Tüm kütüphaneler Arduino Library Manager üzerinden doğrulanmıştır.

🚀 Kurulum

Arduino IDE’yi kur

ESP32 Board Manager ekle

Gerekli kütüphaneleri yükle

Kodu yükle

Seri hızı: 115200 baud

⚠️ Yasal & Güvenlik Uyarısı

Bu proje:

Eğitim

Deneysel

Prototip

amaçlıdır.

Gerçek araçlara doğrudan entegre edilmeden önce:

Elektriksel izolasyon

Otomotiv standartları

EMC testleri

yapılmalıdır.

👤 Geliştirici

Toprak Ahmet Aydoğmuş
Siber Güvenlik & Gömülü Sistemler

🔗 GitHub: https://github.com/toprakahmetaydogmus

🔗 LinkedIn: https://www.linkedin.com/in/toprak-ahmet-aydoğmuş-60462534b/

⭐ Katkı & Geliştirme

Pull request’ler, iyileştirmeler ve yeni fikirler memnuniyetle karşılanır.
Bu proje yaşayan bir sistem olarak tasarlanmıştır.

# full-car-safety-system
car assistant, esp32, iot, embedded systems,  smart vehicle, arduino, sensor, security, automotive,  turkish project, open source, electronics, robotics
