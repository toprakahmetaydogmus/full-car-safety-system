# 🚗 TOPRAK — Akıllı Araç Güvenlik ve Telemetri Asistanı (V10.0 PRO MAX)

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32%20NodeMCU-brightgreen.svg)](#)
[![Version: V10.0](https://img.shields.io/badge/Version-V10.0%20PRO%20MAX-blue.svg)](#)
[![Display: SSD1306 OLED](https://img.shields.io/badge/OLED-128x64%20SSD1306-orange.svg)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Geliştirici: **Toprak Ahmet Aydoğmuş**

---

## 🎯 1. Proje Özeti

**TOPRAK Akıllı Araba Asistanı (V10.0 PRO MAX)**, ESP32 mikrodenetleyicisi üzerinde çalışan; ultrasonik radar proximity taraması, hava kalitesi/gaz tespiti (MQ-135/MQ-2), hareket/güvenlik algılama (PIR/RCWL-0507), joystick kontrollü 180° SG90 servo motor, non-blocking sesli/görsel uyarı sistemi ve Bluetooth telemetri bağlantısına sahip yeni nesil gömülü otomotiv güvenlik platformudur.

---

## 🛠️ 2. V10.0 Sürümünde Giderilen Kritik Sorunlar

| Sorun | Eski Davranış | V10.0 Çözümü |
|---|---|---|
| **Menünün Otomatik Aşağı Kayması** | Joystick analog okuması eşik üstünde kaldığında kontrolsüzce kayıyordu. | **Boot anında otomatik merkez kalibrasyonu** ve **Edge-Trigger** mantığı eklendi. Joystick merkeze bırakılmadan asla ikinci adım atmaz. |
| **Gaz Sensörünün Joystick Hareketinde Dalgalanması (ADC Crosstalk)** | Joystick hareket ettirildiğinde SAR ADC1 S&H kapasitör yükü GPIO 36'ya sızıyor, gaz değeri yapay şekilde fırlıyordu. | **ADC kanal geçiş izolasyonu (Dummy read + 300µs yerleşme süresi)**, 7 örnekli **Median/Outlier filtresi** ve 35ms joystick periyodu eklendi. Joystick ile gaz sensörü tamamen izole edildi; gaz okuması çevik ve stabil hale getirildi. |
| **Buzzer'ın Mesafe & Gaz Uyarısı** | Mesafe uyarıları sabit veya aşırı sesliydi. | **Kademeli ve Akıllı İkaz**: $\le 20\text{ cm}$ ve Gaz Tehlikesinde **Aşırı Sesli acil alarm (tam güç, seri flaşör)**; $20 - 50\text{ cm}$ arasında **mesafe azaldıkça hızlanan ve yükselen kademeli park bipi**; $> 50\text{ cm}$ mesafede **tamamen sessiz**. |
| **Servonun Sağa Takılı Kalması** | Timer 0 çakışması servoyu 880Hz ile sağa kilitliyordu. | **4 Donanımsal Timer (Timer 0-3)** ayrıldı. SG90 mekanik dişli kilitlenmesini önlemek için **20° - 160° arasında kesintisiz, akıcı sol-sağ arka plan taraması** etkinleştirildi. |
| **Joystick Orta Tuşu İle Geri Dönüş** | Alt menülerden geri dönmek için sadece D18 butonu gerekiyordu. | **Joystick Orta Tuşuna (SW) basıldığında** alt menülerden anında Ana Menüye geri dönme özelliği eklendi. Ana menüdeyken ise seçili sayfaya girer. |
| **Menünün Ekranda Kaybolması** | Boş ekranlı uyku modu ve kontrolsüz ekran temizleme menüyü siliyordu. | Ekranı kapatan uyku modu kaldırıldı; menü ve ekranlar her zaman net ve sabit kalır. |
| **Ekrana Sığmama ve Yazıların Karışması** | Metin boyutları 128 pikseli aşıyor, alt bilgi çubuğu ile yazılar çakışıyordu. | **Piksel-hassas 128x64 dual-color layout**: Üst sarı bilgi çubuğu (0-14px), ana içerik (16-52px), alt navigasyon ve animasyonlu robot (53-63px) olarak ayrıldı. Çakışma sıfırlandı. |
| **Servonun Tam Dönmemesi** | Servonun hareket sınırları kısıtlıydı ve sadece belirli menülerde dönüyordu. | 0° ile 180° arasında tam açılı süpürme sağlandı. Kullanıcının mekanik montajı için **66° orta nokta** ve **Ters Joystick** desteği korundu. |
| **ESP32 Core 3.x Uyumluluk Hatası** | `ledcSetup` ve `ledcAttachPin` fonksiyonları yeni derleyicilerde derleme hatası veriyordu. | Core 2.x ve Core 3.x hibrit PWM/LEDC motoru yazılarak derleme hatası giderildi. |
| **Tek Tuşla Sessize Alma (Mute)** | Ses kapatmak için menüde gezinmek gerekiyordu. | **D18 (Geri) butonuna 1.5 saniye basılı tutulduğunda** anında ses açılır/kapanır. |

---

## 🔌 3. Donanım Bağlantı Şeması (Tamamen Korundu)

| Bileşen | ESP32 Pin | Açıklama |
|---|---|---|
| **OLED SDA** | GPIO 21 | I2C Veri Hattı (SSD1306 128x64) |
| **OLED SCL** | GPIO 22 | I2C Saat Hattı |
| **Joystick VRx** | GPIO 34 | Analog X Ekseni (Servo sağ/sol yönlendirme) |
| **Joystick VRy** | GPIO 35 | Analog Y Ekseni (Menü yukarı/aşağı gezinme) |
| **Joystick SW (Buton)** | GPIO 32 | Dahili Pull-Up (Seç / Ayar Düzenle / Merkeze Al) |
| **Geri Butonu (D18)** | GPIO 18 | Dahili Pull-Up (Kısa basma: Geri / Uzun basma: Sessiz Mod) |
| **HC-SR04 TRIG** | GPIO 13 | Ultrasonik Tetikleme |
| **HC-SR04 ECHO** | GPIO 12 | Ultrasonik Yankı Girişi |
| **MQ-135 / MQ-2** | GPIO 36 (VP) | Analog Gaz / Hava Kalitesi Girişi |
| **PIR / RCWL-0507** | GPIO 14 | Hareket / Güvenlik Algılama |
| **Buzzer** | GPIO 15 | PWM Sesli İkaz (Farklı ton & ritimler) |
| **Sistem LED** | GPIO 2 | ESP32 Dahili Kalp Atışı LED'i |
| **Uyarı LED / Şerit** | GPIO 25 | PWM Flaşör / Parlaklık Göstergesi |
| **SG90 Servo Motor** | GPIO 26 | Radar Tarama Motoru (50 Hz PWM) |

---

## 🕹️ 4. Menü ve Kontrol Rehberi

### Kontrol Butonları
- **Joystick Yukarı / Aşağı:** Menü öğeleri arasında gezinir (Edge-triggered: 1 itme = 1 adım).
- **Joystick Butonu (Basma):** Menüye girer / Ayar ekranında düzenleme modunu açar-kapatır / Servo ekranında 66° merkeze alır.
- **Joystick Sol / Sağ:** Ayar değerlerini hassasça artırır / azaltır.
- **D18 Butonu (Kısa Basma):** Her zaman bir önceki menüye / Ana Menüye döner.
- **D18 Butonu (1.5 sn Uzun Basma):** Sistemi anında **SESSİZ MODA (Mute)** alır veya sesi tekrar açar.

### Ana Menü Yapısı
1. **Mesafe Radarı:** Büyük font anlık cm ölçümü, görsel yakınlık barı ve mesafe uyarı durumu.
2. **Gaz & Hava:** Canlı PPM değeri, gaz seviye barı, ham ADC okuması ve hava kalitesi sınıflandırması (*Temiz / Orta / Tehlikeli*).
3. **Hareket & Güvenlik:** PIR/Mikrodalga hareket algılama durumu, toplam sayım ve güvenlik alarmı modu.
4. **Radar Tarama:** 180° yarım çember radar ekranı; anlık dönen tarama ışını ve tespit edilen engel noktaları.
5. **Manuel Servo:** Joystick ile servoyu 0° - 180° arasında doğrudan yönlendirme, butona basıldığında 66° merkeze dönüş.
6. **İstatistikler:** Çalışma süresi (uptime), okunan min/max/ort mesafeler, boş RAM miktarı.
7. **Mesafe Ayarları:** Uyarı eşiği (cm), Tehlike eşiği (cm) ve maksimum menzil ayarı.
8. **Gaz Ayarları:** Gaz alarm eşiği (PPM) ayarı ve anlık kalibrasyon.
9. **Servo Ayarları:** Merkez açısı (varsayılan: 66°), Ters Joystick (Açık/Kapalı) ve tarama hızı.
10. **Sistem & Ses:** Sesli ikaz (Açık/Kapalı), Uyarı LED'i ve Fabrika Ayarlarına Dönüş.

---

## 📱 5. Bluetooth Telemetri ve Uzaktan Kontrol

ESP32 açıldığında otomatik olarak **`TOPRAK-V10`** adıyla Bluetooth yayını yapar. Telefonunuzdan herhangi bir *Serial Bluetooth Terminal* uygulaması ile bağlanabilirsiniz.

### JSON Telemetri Formatı (1 Hz Otomatik Yayın)
```json
{"dist": 45.2, "gas": 420, "motion": 0, "angle": 66, "alert": 0}
```

### Bluetooth Üzerinden Gönderilebilecek Komutlar
- `GET` : Anlık sensör verilerini ve durumunu JSON formatında döndürür.
- `SERVO:90` : Servoyu istenen açıya döndürür (0 - 180).
- `SCAN:1` : Otomatik radar taramasını başlatır.
- `SCAN:0` : Otomatik radar taramasını durdurur.
- `MUTE` : Sesi açar veya kapatır.

---

## ⚡ 6. Kurulum ve Yükleme (Arduino IDE / CLI)

### Arduino IDE Ayarları
1. **Kart:** `ESP32 Dev Module`
2. **Flash Mode:** `DIO` *(Önemli: QIO seçilirse ESP32 boot etmeyebilir)*
3. **Flash Frequency:** `40MHz` veya `80MHz`
4. **Partition Scheme:** `Default 4MB with spiffs`
5. **Upload Speed:** `921600` veya `115200`

### Arduino CLI ile Yükleme
```bash
arduino-cli compile --upload -p COM4 --fqbn esp32:esp32:esp32:FlashMode=dio,FlashFreq=40 v10.ino
```

---

## 📜 Lisans
Bu proje [MIT Lisansı](LICENSE) kapsamında açık kaynak olarak yayınlanmıştır.  
Geliştirici: **Toprak Ahmet Aydoğmuş**.
