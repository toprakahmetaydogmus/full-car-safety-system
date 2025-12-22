/****************************************************
 * TOPRAK - Akıllı Araba Asistanı PRO MAX V5.0
 * TAM GELİŞTİRİLMİŞ SON SÜRÜM
 * 
 * YENİ ÖZELLİKLER:
 * 1. ÇOK HIZLI SERVO (5ms)
 * 2. JOYSTICK İLE MANUEL SERVO KONTROLÜ
 * 3. GELİŞTİRİLMİŞ HC-SR04 ALGORİTMASI
 * 4. MESAFEYE GÖRE LED PARLAKLIK
 * 5. BEEP SORUNU DÜZELTİLDİ
 ****************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// =========== PIN TANIMLARI ===========
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C

#define JOY_X 34
#define JOY_Y 35
#define JOY_BTN 32
#define GERI_BUTON 18

#define TRIG_PIN 13
#define ECHO_PIN 12
#define GAS_PIN 36
#define PIR_PIN 14
#define BUZZER_PIN 15
#define SYS_LED 2
#define SERVO_PIN 26
#define LED_PIN 25    // TEK BEYAZ LED - PWM

// =========== NESNELER ===========
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo taramaServo;

// =========== DEĞİŞKENLER ===========
// Sensörler
float mesafe = 0;
float gaz = 0;

// HC-SR04 Geliştirilmiş
float mesafeFiltreli = 0;
float mesafeBuffer[10] = {0}; // 10 örnekli buffer
int bufferIndex = 0;
unsigned long sonMesafeOkuma = 0;
const int MESAFE_OKUMA_ARALIGI = 80; // ms

// PIR Hareket Sensörü
bool hareket = false;
unsigned long sonHareketZamani = 0;
int hareketSayisi = 0;
bool hareketTespitEdildi = false;
int hareketSikligi = 0;
unsigned long sonHareketAnaliz = 0;

// PIR Filtreleme
int pirOkumaSayaci = 0;
const int PIR_ESIK = 4;
bool lastPirState = false;
unsigned long sonPirOkuma = 0;
const int PIR_OKUMA_ARALIGI = 100;
int hareketSure = 0;

// PIR Kalibrasyon
bool pirKalibrasyonYapiliyor = false;
unsigned long pirKalibrasyonBaslangic = 0;

// Butonlar
bool joyBtn = false, lastJoyBtn = false;
bool geriBtn = false, lastGeriBtn = false;

// Menü Sistemi
#define MENU_ANA 0
#define MENU_MESAFE 1
#define MENU_GAZ 2
#define MENU_HAREKET 3
#define MENU_TARAMA 4
#define MENU_SERVO_MANUEL 5    // YENİ: Joystick ile servo kontrol
#define MENU_AYAR_MESAFE 6
#define MENU_AYAR_GAZ 7
#define MENU_AYAR_HAREKET 8
#define MENU_AYAR_SES 9
#define MENU_AYAR_SERVO 10
#define MENU_AYAR_LED 11
#define MENU_SISTEM 12
#define MENU_GUVENLIK 13
#define MENU_OTONOM 14
#define MENU_AYAR_PIR 15

int aktifMenu = MENU_ANA;
int menuSecim = 0;
bool ayarModu = false;
int ayarSecim = 0;
bool manuelServoMode = false; // Joystick ile servo kontrol modu

int menuBaslangic = 0;
const int MAX_GORUNEN_OGE = 6;

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0;
unsigned long sonUyariZamani = 0;
bool beepCalisiyor = false;

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0;

// Servo Sistemi - ÇOK HIZLI
int servoAci = 90;
int hedefAci = 90;
bool taramaYonu = true;
bool taramaAktif = true;
int taramaHizi = 5; // ÇOK HIZLI (5ms)
float taramaHaritasi[37] = {0};
unsigned long sonServoHareket = 0;
bool alarmModu = false;
int alarmServoHizi = 10; // Alarmda daha da hızlı

// LED Sistemi - Mesafeye göre parlaklık
int ledParlaklik = 150;
int ledMod = 2; // Default: Mesafe feedback modu
bool ledDurum = true;
unsigned long sonLedDegisim = 0;

// Güvenlik Sistemi
bool guvenlikAktif = false;
bool alarmCalisiyor = false;
unsigned long alarmBaslangic = 0;
int alarmSeviye = 0;

// Otonom Mod
bool otonomMod = false;
int otonomDurum = 0;
unsigned long sonOtonomKarar = 0;

// Joystick Servo Kontrol
int joyServoSpeed = 15; // Joystick servo hızı
unsigned long sonJoyServoHareket = 0;

// Ayarlar
struct {
  int mesafeDikkat = 50;
  int mesafeTehlike = 20;
  int gazEsik = 800;
  int hareketEsikSure = 3000;
  bool ses = true;
  
  bool servoAktif = true;
  int servoSpeed = 5; // ÇOK HIZLI
  int servoMinAci = 10;
  int servoMaxAci = 170;
  
  bool ledAktif = true;
  int ledMode = 2; // Default: Mesafe feedback
  int ledHiz = 500;
  int ledBright = 150;
  
  bool guvenlikOtomatik = false;
  int guvenlikHassasiyet = 2;
  int alarmSuresi = 10000;
  
  int pirHassasiyet = 2;
  bool pirFiltre = true;
  
  bool robotAnimasyon = true;
  
  bool otonomAktif = false;
  int otonomHiz = 30;
  
  // Yeni ayarlar
  bool mesafeFiltre = true;
  int mesafeOrtalama = 5;
  int joyServoHiz = 15; // Joystick servo hızı
} ayar;

// Zamanlayıcılar
unsigned long sonOkuma = 0;
unsigned long sonBip = 0;
unsigned long sonEtkilesim = 0;
bool uykuModu = false;

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  
  // Pinler
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(GERI_BUTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SYS_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(SYS_LED, LOW);
  
  // Servo - ÇOK HIZLI, GENİŞ PWM ARALIĞI
  taramaServo.attach(SERVO_PIN, 500, 2500);
  Serial.println("Servo testi basliyor...");
  
  // Hızlı servo test
  taramaServo.write(0);
  delay(100);
  taramaServo.write(180);
  delay(100);
  taramaServo.write(90);
  delay(100);
  
  servoAci = 90;
  hedefAci = 90;
  
  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED hatasi!");
    while(1);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Başlangıç
  baslangic();
  Serial.println("TOPRAK PRO MAX V5.0 - Sistem hazir!");
  Serial.print("Servo hizi: ");
  Serial.print(ayar.servoSpeed);
  Serial.println("ms");
  Serial.print("LED Modu: ");
  Serial.println(ayar.ledMode);
}

void baslangic() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPRAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("PRO MAX V5.0");
  display.display();
  
  // LED test - Mesafe feedback modunda
  if(ayar.ledAktif) {
    Serial.println("LED test basliyor (mesafe feedback)...");
    
    // Mesafe feedback testi
    for(int i = 400; i > 0; i -= 20) {
      int parlaklik = map(i, 0, 100, 255, 10);
      parlaklik = constrain(parlaklik, 10, 255);
      analogWrite(LED_PIN, parlaklik);
      delay(20);
    }
    
    // Varsayılan parlaklık
    analogWrite(LED_PIN, ledParlaklik);
  }
  
  delay(1000);
}

// =========== HC-SR04 GELİŞTİRİLMİŞ OKUMA ===========
float hcSr04Oku() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(12); // 12µs daha iyi sonuç veriyor
  digitalWrite(TRIG_PIN, LOW);
  
  // pulseIn yerine daha güvenilir okuma
  unsigned long timeout = micros() + 30000; // 30ms timeout
  unsigned long startTime, endTime;
  
  // Echo pinini bekle
  while(digitalRead(ECHO_PIN) == LOW && micros() < timeout);
  startTime = micros();
  
  while(digitalRead(ECHO_PIN) == HIGH && micros() < timeout);
  endTime = micros();
  
  if(micros() >= timeout) {
    return 400.0; // Timeout
  }
  
  long sure = endTime - startTime;
  float mesafeCm = sure * 0.0343 / 2.0; // Daha hassas sabit
  
  // Geçersiz değerleri filtrele
  if(mesafeCm < 2.0 || mesafeCm > 400.0) {
    return 400.0;
  }
  
  return mesafeCm;
}

float hcSr04OkuFiltreli() {
  static float sonGecerliMesafe = 100.0;
  static float sonTrend = 0;
  
  // 3 hızlı okuma yap
  float okumalar[3];
  for(int i = 0; i < 3; i++) {
    okumalar[i] = hcSr04Oku();
    delay(2);
  }
  
  // Medyan filtresi
  float medyan;
  if(okumalar[0] > okumalar[1]) {
    if(okumalar[1] > okumalar[2]) medyan = okumalar[1];
    else if(okumalar[0] > okumalar[2]) medyan = okumalar[2];
    else medyan = okumalar[0];
  } else {
    if(okumalar[0] > okumalar[2]) medyan = okumalar[0];
    else if(okumalar[1] > okumalar[2]) medyan = okumalar[2];
    else medyan = okumalar[1];
  }
  
  // Hareketli ortalama filtresi
  mesafeBuffer[bufferIndex] = medyan;
  bufferIndex = (bufferIndex + 1) % 10;
  
  // Buffer'daki geçerli değerlerin ortalamasını al
  float toplam = 0;
  int gecerliSayisi = 0;
  for(int i = 0; i < 10; i++) {
    if(mesafeBuffer[i] > 2.0 && mesafeBuffer[i] < 400.0) {
      toplam += mesafeBuffer[i];
      gecerliSayisi++;
    }
  }
  
  if(gecerliSayisi > 0) {
    float ortalama = toplam / gecerliSayisi;
    
    // Ani değişimleri filtrele
    float fark = abs(ortalama - sonGecerliMesafe);
    if(fark < 50.0 || sonGecerliMesafe == 100.0) {
      sonGecerliMesafe = ortalama;
    }
    
    // Trend filtresi
    float trendFiltre = 0.7; // %70 yeni değer, %30 trend
    float filtrelenmis = (ortalama * trendFiltre) + (sonGecerliMesafe * (1.0 - trendFiltre));
    
    return filtrelenmis;
  }
  
  return sonGecerliMesafe;
}

// =========== PIR KALİBRASYON ===========
void pirKalibrasyonBaslat() {
  pirKalibrasyonYapiliyor = true;
  pirKalibrasyonBaslangic = millis();
  pirOkumaSayaci = 0;
  Serial.println("PIR kalibrasyon baslatildi. 5 saniye hareket etmeyin...");
  
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("PIR KALIBRASYON");
  display.setCursor(10, 40);
  display.print("Lutfen bekleyin...");
  display.display();
}

void pirKalibrasyonYap() {
  if (!pirKalibrasyonYapiliyor) return;
  
  unsigned long simdi = millis();
  
  if (simdi - pirKalibrasyonBaslangic > 5000) {
    pirKalibrasyonYapiliyor = false;
    pirOkumaSayaci = 0;
    Serial.println("PIR kalibrasyon tamamlandi!");
    
    display.clearDisplay();
    display.setCursor(10, 20);
    display.print("KALIBRASYON");
    display.setCursor(10, 40);
    display.print("TAMAMLANDI!");
    display.display();
    delay(1000);
  }
}

// =========== PIR OKUMA FONKSİYONU ===========
bool pirOkuFiltreli() {
  unsigned long simdi = millis();
  
  if(simdi - sonPirOkuma < PIR_OKUMA_ARALIGI) {
    return hareket;
  }
  
  sonPirOkuma = simdi;
  bool yeniOkuma = (digitalRead(PIR_PIN) == HIGH);
  
  if (pirKalibrasyonYapiliyor) {
    return false;
  }
  
  int esikArtis = 1;
  int esikAzalis = 1;
  
  switch(ayar.pirHassasiyet) {
    case 1:
      esikArtis = 1;
      esikAzalis = 2;
      break;
    case 2:
      esikArtis = 2;
      esikAzalis = 1;
      break;
    case 3:
      esikArtis = 3;
      esikAzalis = 1;
      break;
  }
  
  if(yeniOkuma) {
    pirOkumaSayaci += esikArtis;
  } else {
    pirOkumaSayaci -= esikAzalis;
  }
  
  pirOkumaSayaci = constrain(pirOkumaSayaci, 0, 10);
  
  bool yeniHareket = false;
  
  if(ayar.pirFiltre) {
    yeniHareket = (pirOkumaSayaci >= 6);
  } else {
    yeniHareket = (pirOkumaSayaci >= 3);
  }
  
  if(yeniHareket && !lastPirState) {
    delay(20);
    bool dogrulama = (digitalRead(PIR_PIN) == HIGH);
    if(!dogrulama) {
      yeniHareket = false;
      pirOkumaSayaci = 2;
    }
  }
  
  lastPirState = yeniHareket;
  return yeniHareket;
}

// =========== SENSÖR OKUMA ===========
void sensorOku() {
  unsigned long simdi = millis();
  
  // HC-SR04 okuma (80ms'de bir)
  if(simdi - sonMesafeOkuma >= MESAFE_OKUMA_ARALIGI) {
    mesafe = hcSr04OkuFiltreli();
    sonMesafeOkuma = simdi;
  }
  
  // Gaz okuma
  int gazDeger = analogRead(GAS_PIN);
  gaz = map(gazDeger, 0, 4095, 0, 2000);
  
  // PIR Kalibrasyon
  if (pirKalibrasyonYapiliyor) {
    pirKalibrasyonYap();
    return;
  }
  
  // PIR Okuma
  bool yeniHareket = pirOkuFiltreli();
  
  if(yeniHareket && !hareket) {
    hareket = true;
    sonHareketZamani = millis();
    hareketSayisi++;
    hareketTespitEdildi = true;
    robotIfade = 5;
    
    if(guvenlikAktif) {
      alarmSeviye = 1;
    }
  } 
  else if(!yeniHareket && hareket) {
    hareket = false;
    hareketSure = millis() - sonHareketZamani;
    
    if(hareketSure > 3000 && guvenlikAktif) {
      alarmSeviye = 2;
      alarmCalisiyor = true;
      alarmBaslangic = millis();
    }
  }
  
  if(hareketTespitEdildi && millis() - sonHareketZamani > 2000) {
    hareketTespitEdildi = false;
  }
  
  if(millis() - sonHareketAnaliz > 10000) {
    hareketSikligi = hareketSayisi;
    hareketSayisi = 0;
    sonHareketAnaliz = millis();
  }
}

// =========== UYARI KONTROL ===========
void uyariKontrol() {
  bool oncekiUyari = uyariVar;
  int oncekiSeviye = uyariSeviye;
  
  uyariVar = false;
  uyariSeviye = 0;
  
  if(mesafe < ayar.mesafeTehlike) {
    uyariVar = true;
    uyariSeviye = 2;
    robotIfade = 2;
    sonUyariZamani = millis();
  } else if(mesafe < ayar.mesafeDikkat) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
    sonUyariZamani = millis();
  } else if(gaz > ayar.gazEsik) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
    sonUyariZamani = millis();
  } else if(hareketTespitEdildi && guvenlikAktif && hareketSure > 600) {
    uyariVar = true;
    uyariSeviye = alarmSeviye;
    robotIfade = 5;
    sonUyariZamani = millis();
  } else {
    robotIfade = 0;
  }
  
  // Uyku modu
  if(uykuModu) {
    robotIfade = 4;
  }
  
  // Uyarı durumu değiştiyse
  if((oncekiUyari != uyariVar) || (oncekiSeviye != uyariSeviye)) {
    if(!uyariVar && ayar.ses) {
      noTone(BUZZER_PIN);
      beepCalisiyor = false;
      sonBip = 0;
    }
  }
  
  digitalWrite(SYS_LED, uyariVar);
}

// =========== SERVO KONTROLÜ - ÇOK HIZLI ===========
void servoKontrol() {
  unsigned long simdi = millis();
  
  if(servoAci != hedefAci) {
    // Çok hızlı hareket (direkt atama)
    servoAci = hedefAci;
    taramaServo.write(servoAci);
  } else {
    // Yeni hedef açı belirle - 10 derece adımlarla (daha hızlı)
    if(taramaYonu) {
      hedefAci += 10;
      if(hedefAci > ayar.servoMaxAci) {
        hedefAci = ayar.servoMaxAci;
        taramaYonu = false;
      }
    } else {
      hedefAci -= 10;
      if(hedefAci < ayar.servoMinAci) {
        hedefAci = ayar.servoMinAci;
        taramaYonu = true;
      }
    }
    
    // Tarama haritasını güncelle
    int taramaIndex = map(servoAci, 0, 180, 0, 36);
    taramaIndex = constrain(taramaIndex, 0, 36);
    taramaHaritasi[taramaIndex] = mesafe;
  }
}

void alarmServoKontrol(unsigned long simdi) {
  static unsigned long sonAlarmHareket = 0;
  static bool alarmYonu = true;
  
  if(simdi - sonAlarmHareket > alarmServoHizi) {
    if(alarmYonu) {
      servoAci += 20; // Alarmda çok hızlı hareket
      if(servoAci > 170) {
        servoAci = 170;
        alarmYonu = false;
      }
    } else {
      servoAci -= 20;
      if(servoAci < 10) {
        servoAci = 10;
        alarmYonu = true;
      }
    }
    taramaServo.write(servoAci);
    sonAlarmHareket = simdi;
  }
}

// =========== JOYSTICK İLE SERVO KONTROL ===========
void joyServoKontrol() {
  unsigned long simdi = millis();
  
  if(simdi - sonJoyServoHareket < joyServoSpeed) {
    return;
  }
  
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  
  // Joystick X ekseni ile servo kontrolü
  if(joyX < 1000) { // SOL
    servoAci -= 5;
    if(servoAci < ayar.servoMinAci) servoAci = ayar.servoMinAci;
    taramaServo.write(servoAci);
    sonJoyServoHareket = simdi;
  } 
  else if(joyX > 3000) { // SAĞ
    servoAci += 5;
    if(servoAci > ayar.servoMaxAci) servoAci = ayar.servoMaxAci;
    taramaServo.write(servoAci);
    sonJoyServoHareket = simdi;
  }
  
  // Joystick butonu ile merkeze dön
  if(digitalRead(JOY_BTN) == LOW) {
    servoAci = 90;
    taramaServo.write(servoAci);
    delay(100);
  }
}

// =========== LED KONTROLÜ - MESAFEYE GÖRE PARLAKLIK ===========
void ledKontrol(unsigned long simdi) {
  if(!ayar.ledAktif) {
    analogWrite(LED_PIN, 0);
    return;
  }
  
  switch(ledMod) {
    case 0: // Sabit
      analogWrite(LED_PIN, ledParlaklik);
      break;
      
    case 1: // Yanıp sönme
      if(simdi - sonLedDegisim > ayar.ledHiz) {
        ledDurum = !ledDurum;
        analogWrite(LED_PIN, ledDurum ? ledParlaklik : 0);
        sonLedDegisim = simdi;
      }
      break;
      
    case 2: // Mesafe feedback - GELİŞTİRİLMİŞ
      {
        // Mesafe azaldıkça parlaklık ARTSIN
        int yeniParlaklik;
        
        if(mesafe >= 200) {
          yeniParlaklik = 5; // Çok uzak - çok karanlık
        } else if(mesafe >= 100) {
          yeniParlaklik = map(mesafe, 100, 200, 30, 5);
        } else if(mesafe >= 50) {
          yeniParlaklik = map(mesafe, 50, 100, 80, 30);
        } else if(mesafe >= 20) {
          yeniParlaklik = map(mesafe, 20, 50, 180, 80);
        } else if(mesafe >= 5) {
          yeniParlaklik = map(mesafe, 5, 20, 255, 180);
        } else {
          yeniParlaklik = 255; // Çok yakın - maksimum parlaklık
        }
        
        yeniParlaklik = constrain(yeniParlaklik, 5, 255);
        
        // Ani değişimleri filtrele
        static int oncekiParlaklik = 150;
        int fark = abs(yeniParlaklik - oncekiParlaklik);
        
        if(fark > 10) {
          // Yumuşak geçiş
          if(yeniParlaklik > oncekiParlaklik) {
            oncekiParlaklik += 10;
          } else {
            oncekiParlaklik -= 10;
          }
          oncekiParlaklik = constrain(oncekiParlaklik, 5, 255);
        } else {
          oncekiParlaklik = yeniParlaklik;
        }
        
        analogWrite(LED_PIN, oncekiParlaklik);
      }
      break;
      
    case 3: // Hareket feedback
      if(hareketTespitEdildi) {
        // Hareket varsa hızlı yanıp sön
        if(simdi - sonLedDegisim > 100) {
          ledDurum = !ledDurum;
          analogWrite(LED_PIN, ledDurum ? ledParlaklik : 0);
          sonLedDegisim = simdi;
        }
      } else {
        // Hareket yoksa sabit
        analogWrite(LED_PIN, ledParlaklik);
      }
      break;
  }
}

// =========== SES KONTROLÜ - GELİŞTİRİLMİŞ ===========
void sesLED() {
  if(!uyariVar || !ayar.ses || beepCalisiyor) return;
  
  unsigned long simdi = millis();
  
  // Uyarı 2 saniyeden eskiyse bip sesini durdur
  if(simdi - sonUyariZamani > 2000) {
    return;
  }
  
  int bipAraligi;
  
  if(uyariSeviye == 1) {
    bipAraligi = 1500; // Dikkat seviyesi: 1.5 saniyede bir
  } else {
    bipAraligi = 800; // Tehlike seviyesi: 0.8 saniyede bir
  }
  
  if(simdi - sonBip > bipAraligi) {
    beepCalisiyor = true;
    // Kısa bip sesi (80ms)
    tone(BUZZER_PIN, uyariSeviye == 1 ? 800 : 1200, 80);
    sonBip = simdi;
    
    // Bip bittikten sonra flag'i sıfırla
    delay(100);
    beepCalisiyor = false;
  }
}

// =========== ANA DÖNGÜ ===========
void loop() {
  unsigned long simdi = millis();
  
  // Sensör okuma
  if(simdi - sonOkuma > 50) {
    sensorOku();
    uyariKontrol();
    sonOkuma = simdi;
  }
  
  // Buton kontrol
  butonOku();
  butonIsle();
  
  // Joystick kontrol (manuel modda değilse)
  if(!manuelServoMode) {
    joyKontrol();
  }
  
  // Servo kontrol
  if(ayar.servoAktif) {
    if(manuelServoMode) {
      // Joystick ile manuel kontrol
      joyServoKontrol();
    } else if(alarmModu && alarmCalisiyor) {
      alarmServoKontrol(simdi);
    } else if(taramaAktif && simdi - sonServoHareket > taramaHizi) {
      servoKontrol();
      sonServoHareket = simdi;
    }
  }
  
  // LED kontrol
  ledKontrol(simdi);
  
  // Ekran
  ekranCiz();
  
  // Ses kontrol
  sesLED();
  
  // Animasyon
  if(ayar.robotAnimasyon && simdi - sonGozKirpma > 3000 + random(0, 2000)) {
    gozAcik = !gozAcik;
    sonGozKirpma = simdi;
  }
  
  // Uyku modu
  if(simdi - sonEtkilesim > 20000 && !uykuModu && !guvenlikAktif) {
    uykuModu = true;
    robotIfade = 4;
  } else if(simdi - sonEtkilesim < 20000 && uykuModu) {
    uykuModu = false;
    robotIfade = 0;
  }
  
  // Küçük bekleme
  delay(10);
}

// =========== BUTON KONTROL ===========
void butonOku() {
  joyBtn = (digitalRead(JOY_BTN) == LOW);
  geriBtn = (digitalRead(GERI_BUTON) == LOW);
}

void butonIsle() {
  static unsigned long sonTiklama = 0;
  unsigned long simdi = millis();
  
  if(simdi - sonTiklama < 300) return;
  
  // D18 - HER ZAMAN ANA MENÜ
  if(geriBtn && !lastGeriBtn) {
    aktifMenu = MENU_ANA;
    ayarModu = false;
    manuelServoMode = false;
    menuSecim = 0;
    menuBaslangic = 0;
    ayarSecim = 0;
    robotIfade = 3;
    bipSesi(2);
    sonEtkilesim = simdi;
    uykuModu = false;
    sonTiklama = simdi;
  }
  
  // JOYSTICK BUTON
  if(joyBtn && !lastJoyBtn) {
    sonEtkilesim = simdi;
    uykuModu = false;
    
    switch(aktifMenu) {
      case MENU_ANA:
        switch(menuSecim) {
          case 0: aktifMenu = MENU_MESAFE; break;
          case 1: aktifMenu = MENU_GAZ; break;
          case 2: aktifMenu = MENU_HAREKET; break;
          case 3: aktifMenu = MENU_TARAMA; break;
          case 4: aktifMenu = MENU_SERVO_MANUEL; manuelServoMode = true; break; // Manuel servo modu
          case 5: aktifMenu = MENU_AYAR_MESAFE; break;
          case 6: aktifMenu = MENU_AYAR_GAZ; break;
          case 7: aktifMenu = MENU_AYAR_HAREKET; break;
          case 8: aktifMenu = MENU_AYAR_SES; break;
          case 9: aktifMenu = MENU_AYAR_SERVO; break;
          case 10: aktifMenu = MENU_AYAR_LED; break;
          case 11: aktifMenu = MENU_SISTEM; break;
          case 12: aktifMenu = MENU_GUVENLIK; break;
          case 13: aktifMenu = MENU_OTONOM; break;
          case 14: aktifMenu = MENU_AYAR_PIR; break;
        }
        break;
        
      case MENU_TARAMA:
        taramaAktif = !taramaAktif;
        break;
        
      case MENU_SERVO_MANUEL:
        // Manuel modda joystick butonu merkeze getirir
        servoAci = 90;
        taramaServo.write(servoAci);
        break;
        
      case MENU_GUVENLIK:
        guvenlikAktif = !guvenlikAktif;
        if(guvenlikAktif) {
          bipSesi(3);
        } else {
          alarmCalisiyor = false;
          alarmModu = false;
          noTone(BUZZER_PIN);
        }
        break;
        
      case MENU_OTONOM:
        otonomMod = !otonomMod;
        break;
        
      case MENU_AYAR_PIR:
        pirKalibrasyonBaslat();
        break;
        
      case MENU_AYAR_MESAFE:
      case MENU_AYAR_GAZ:
      case MENU_AYAR_HAREKET:
      case MENU_AYAR_SERVO:
      case MENU_AYAR_LED:
        ayarModu = !ayarModu;
        ayarSecim = 0;
        break;
        
      case MENU_AYAR_SES:
        ayar.ses = !ayar.ses;
        if(!ayar.ses) {
          noTone(BUZZER_PIN);
          beepCalisiyor = false;
        }
        break;
    }
    
    bipSesi(1);
    sonTiklama = simdi;
  }
  
  lastJoyBtn = joyBtn;
  lastGeriBtn = geriBtn;
}

void joyKontrol() {
  static unsigned long sonHareket = 0;
  unsigned long simdi = millis();
  
  if(simdi - sonHareket < 200) return;
  
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  bool hareketVar = false;
  
  if(joyY < 1000) {
    hareketVar = true;
    if(aktifMenu == MENU_ANA) {
      menuSecim--;
      if(menuSecim < 0) menuSecim = 14;
      if(menuSecim < menuBaslangic) {
        menuBaslangic = menuSecim;
      } else if(menuSecim >= menuBaslangic + MAX_GORUNEN_OGE) {
        menuBaslangic = menuSecim - MAX_GORUNEN_OGE + 1;
      }
    } else if(ayarModu) {
      ayarSecim--;
    }
  }
  else if(joyY > 3000) {
    hareketVar = true;
    if(aktifMenu == MENU_ANA) {
      menuSecim++;
      if(menuSecim > 14) menuSecim = 0;
      if(menuSecim < menuBaslangic) {
        menuBaslangic = menuSecim;
      } else if(menuSecim >= menuBaslangic + MAX_GORUNEN_OGE) {
        menuBaslangic = menuSecim - MAX_GORUNEN_OGE + 1;
      }
    } else if(ayarModu) {
      ayarSecim++;
    }
  }
  
  if(ayarModu && (joyX > 3000 || joyX < 1000)) {
    hareketVar = true;
    ayarDegistir(joyX > 3000 ? 1 : -1);
  }
  
  if(hareketVar) {
    sonEtkilesim = simdi;
    uykuModu = false;
    bipSesi(1);
    sonHareket = simdi;
  }
}

void ayarDegistir(int yon) {
  switch(aktifMenu) {
    case MENU_AYAR_MESAFE:
      if(ayarSecim == 0) {
        ayar.mesafeDikkat += yon * 5;
        ayar.mesafeDikkat = constrain(ayar.mesafeDikkat, 20, 100);
      } else {
        ayar.mesafeTehlike += yon * 5;
        ayar.mesafeTehlike = constrain(ayar.mesafeTehlike, 5, 50);
      }
      break;
      
    case MENU_AYAR_GAZ:
      ayar.gazEsik += yon * 50;
      ayar.gazEsik = constrain(ayar.gazEsik, 300, 2000);
      break;
      
    case MENU_AYAR_HAREKET:
      if(ayarSecim == 0) {
        ayar.hareketEsikSure += yon * 500;
        ayar.hareketEsikSure = constrain(ayar.hareketEsikSure, 1000, 10000);
      } else if(ayarSecim == 1) {
        ayar.guvenlikHassasiyet += yon;
        ayar.guvenlikHassasiyet = constrain(ayar.guvenlikHassasiyet, 1, 3);
      } else {
        ayar.alarmSuresi += yon * 1000;
        ayar.alarmSuresi = constrain(ayar.alarmSuresi, 5000, 30000);
      }
      break;
      
    case MENU_AYAR_SERVO:
      if(ayarSecim == 0) {
        ayar.servoSpeed += yon * 1;
        ayar.servoSpeed = constrain(ayar.servoSpeed, 1, 20);
        taramaHizi = ayar.servoSpeed;
        Serial.print("Servo Hizi: ");
        Serial.print(ayar.servoSpeed);
        Serial.println("ms");
      } else if(ayarSecim == 1) {
        ayar.servoMinAci += yon * 5;
        ayar.servoMinAci = constrain(ayar.servoMinAci, 0, 175);
      } else if(ayarSecim == 2) {
        ayar.servoMaxAci += yon * 5;
        ayar.servoMaxAci = constrain(ayar.servoMaxAci, 5, 180);
      } else {
        ayar.joyServoHiz += yon * 5;
        ayar.joyServoHiz = constrain(ayar.joyServoHiz, 5, 100);
        joyServoSpeed = ayar.joyServoHiz;
      }
      break;
      
    case MENU_AYAR_LED:
      if(ayarSecim == 0) {
        ayar.ledMode += yon;
        ayar.ledMode = constrain(ayar.ledMode, 0, 3);
        ledMod = ayar.ledMode;
      } else if(ayarSecim == 1) {
        ayar.ledHiz += yon * 50;
        ayar.ledHiz = constrain(ayar.ledHiz, 100, 1000);
      } else {
        ayar.ledBright += yon * 10;
        ayar.ledBright = constrain(ayar.ledBright, 0, 255);
        ledParlaklik = ayar.ledBright;
      }
      break;
  }
}

// =========== EKRAN FONKSİYONLARI ===========
void ekranCiz() {
  display.clearDisplay();
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  baslikCiz();
  icerikCiz();
  robotCiz(110, 50);
  altBilgiCiz();
  display.display();
}

void baslikCiz() {
  display.setCursor(40, 2);
  switch(aktifMenu) {
    case MENU_ANA: display.print("ANA MENU"); break;
    case MENU_MESAFE: display.print("MESAFE"); break;
    case MENU_GAZ: display.print("GAZ"); break;
    case MENU_HAREKET: display.print("HAREKET"); break;
    case MENU_TARAMA: display.print("TARAMA"); break;
    case MENU_SERVO_MANUEL: display.print("SERVO KONTROL"); break;
    case MENU_AYAR_MESAFE: display.print("AYAR MESAFE"); break;
    case MENU_AYAR_GAZ: display.print("AYAR GAZ"); break;
    case MENU_AYAR_HAREKET: display.print("AYAR HAREKET"); break;
    case MENU_AYAR_SES: display.print("AYAR SES"); break;
    case MENU_AYAR_SERVO: display.print("AYAR SERVO"); break;
    case MENU_AYAR_LED: display.print("AYAR LED"); break;
    case MENU_SISTEM: display.print("SISTEM"); break;
    case MENU_GUVENLIK: display.print("GUVENLIK"); break;
    case MENU_OTONOM: display.print("OTONOM MOD"); break;
    case MENU_AYAR_PIR: display.print("PIR AYARI"); break;
  }
}

void icerikCiz() {
  switch(aktifMenu) {
    case MENU_ANA: anaMenuCiz(); break;
    case MENU_MESAFE: mesafeEkranCiz(); break;
    case MENU_GAZ: gazEkranCiz(); break;
    case MENU_HAREKET: hareketEkranCiz(); break;
    case MENU_TARAMA: taramaEkranCiz(); break;
    case MENU_SERVO_MANUEL: servoManuelCiz(); break;
    case MENU_AYAR_MESAFE: ayarMesafeCiz(); break;
    case MENU_AYAR_GAZ: ayarGazCiz(); break;
    case MENU_AYAR_HAREKET: ayarHareketCiz(); break;
    case MENU_AYAR_SES: ayarSesCiz(); break;
    case MENU_AYAR_SERVO: ayarServoCiz(); break;
    case MENU_AYAR_LED: ayarLedCiz(); break;
    case MENU_SISTEM: sistemEkranCiz(); break;
    case MENU_GUVENLIK: guvenlikEkranCiz(); break;
    case MENU_OTONOM: otonomEkranCiz(); break;
    case MENU_AYAR_PIR: ayarPirCiz(); break;
  }
}

void anaMenuCiz() {
  const char* menuItems[] = {
    "Mesafe Goster",
    "Gaz Goster",
    "Hareket Goster",
    "Tarama Ekrani",
    "Servo Manuel Kontrol", // YENİ
    "Mesafe Ayari",
    "Gaz Ayari",
    "Hareket Ayari",
    "Ses Ayari",
    "Servo Ayari",
    "LED Ayari",
    "Sistem Bilgisi",
    "Guvenlik Sistemi",
    "Otonom Mod",
    "PIR Kalibrasyon"
  };
  
  int baslangic = menuBaslangic;
  int bitis = min(baslangic + MAX_GORUNEN_OGE, 15);
  
  for(int i = baslangic; i < bitis; i++) {
    int listIndex = i - baslangic;
    int y = 15 + listIndex * 9;
    
    if(i == menuSecim) {
      display.fillRect(2, y-1, 124, 9, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(4, y);
      display.print(">");
      display.setCursor(12, y);
      display.print(menuItems[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(12, y);
      display.print(menuItems[i]);
    }
  }
  
  if(15 > MAX_GORUNEN_OGE) {
    int scrollBarHeight = 40;
    int scrollBarY = 15;
    int scrollPos = map(menuBaslangic, 0, 15 - MAX_GORUNEN_OGE, scrollBarY, scrollBarY + scrollBarHeight - 5);
    display.drawRect(125, scrollBarY, 2, scrollBarHeight, SSD1306_WHITE);
    display.fillRect(125, scrollPos, 2, 5, SSD1306_WHITE);
  }
}

void mesafeEkranCiz() {
  display.setTextSize(3);
  display.setCursor(15, 15);
  if(mesafe >= 100) display.print(mesafe, 0);
  else if(mesafe >= 10) display.print(mesafe, 0);
  else display.print(mesafe, 1);
  
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("cm");
  
  // Çubuk grafik
  int barGenislik = map(constrain(mesafe, 0, 100), 0, 100, 0, 100);
  display.drawRect(10, 50, 100, 8, SSD1306_WHITE);
  
  if(mesafe < ayar.mesafeTehlike) {
    display.fillRect(10, 50, barGenislik, 8, SSD1306_WHITE);
  } else if(mesafe < ayar.mesafeDikkat) {
    for(int i = 0; i < barGenislik; i += 4) {
      display.drawPixel(10 + i, 53, SSD1306_WHITE);
    }
  } else {
    display.fillRect(10, 50, barGenislik, 8, SSD1306_WHITE);
  }
  
  display.setCursor(5, 60);
  display.print("D:");
  display.print(ayar.mesafeDikkat);
  display.print(" T:");
  display.print(ayar.mesafeTehlike);
}

void gazEkranCiz() {
  display.setTextSize(3);
  display.setCursor(15, 15);
  display.print(gaz, 0);
  
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("ppm");
  
  display.setCursor(5, 50);
  display.print("Kalite: ");
  if(gaz < 500) display.print("IYI");
  else if(gaz < 1000) display.print("ORTA");
  else display.print("KOTU");
  
  display.setCursor(5, 60);
  display.print("Esik: ");
  display.print(ayar.gazEsik);
  display.print("ppm");
}

void hareketEkranCiz() {
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print(hareket ? "VAR" : "YOK");
  
  display.setTextSize(1);
  display.setCursor(5, 45);
  display.print("Sayac: ");
  display.print(pirOkumaSayaci);
  
  display.setCursor(5, 55);
  display.print("Sure: ");
  if(hareket) {
    int gecenSure = (millis() - sonHareketZamani) / 1000;
    display.print(gecenSure);
    display.print(" sn");
  } else {
    display.print("---");
  }
}

void taramaEkranCiz() {
  int merkezX = 64;
  int merkezY = 35;
  int yaricap = 25;
  
  display.drawCircle(merkezX, merkezY, yaricap, SSD1306_WHITE);
  display.drawCircle(merkezX, merkezY, yaricap/2, SSD1306_WHITE);
  
  float radyan = radians(servoAci);
  int bitisX = merkezX + sin(radyan) * yaricap;
  int bitisY = merkezY - cos(radyan) * yaricap;
  display.drawLine(merkezX, merkezY, bitisX, bitisY, SSD1306_WHITE);
  
  for(int i = 0; i < 37; i++) {
    if(taramaHaritasi[i] > 2 && taramaHaritasi[i] < 100) {
      float aci = radians(i * 5);
      float uzaklik = map(taramaHaritasi[i], 0, 100, 0, yaricap);
      int x = merkezX + sin(aci) * uzaklik;
      int y = merkezY - cos(aci) * uzaklik;
      display.fillCircle(x, y, 1, SSD1306_WHITE);
    }
  }
  
  display.setCursor(5, 55);
  display.print("A:");
  display.print(servoAci);
  display.print(" Hiz:");
  display.print(ayar.servoSpeed);
  display.print("ms");
}

void servoManuelCiz() {
  display.setTextSize(2);
  display.setCursor(30, 15);
  display.print("SERVO");
  
  display.setTextSize(3);
  display.setCursor(40, 35);
  display.print(servoAci);
  display.setTextSize(1);
  display.print("°");
  
  display.setCursor(5, 55);
  display.print("JOY Left/Right: Don");
  display.setCursor(5, 60);
  display.print("JOY Button: Merkez");
}

void ayarMesafeCiz() {
  if(ayarModu) {
    display.setCursor(5, 15);
    if(ayarSecim == 0) {
      display.print("Dikkat:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.mesafeDikkat);
      display.print("cm");
    } else {
      display.print("Tehlike:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.mesafeTehlike);
      display.print("cm");
    }
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Dikkat: ");
    display.print(ayar.mesafeDikkat);
    display.print("cm");
    display.setCursor(5, 25);
    display.print("Tehlike: ");
    display.print(ayar.mesafeTehlike);
    display.print("cm");
    display.setCursor(5, 45);
    display.print("Yukari/Asagi: Sec");
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarGazCiz() {
  if(ayarModu) {
    display.setCursor(5, 15);
    display.print("Gaz Esigi:");
    display.setTextSize(2);
    display.setCursor(30, 30);
    display.print(ayar.gazEsik);
    display.print("ppm");
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Mevcut: ");
    display.print(gaz, 0);
    display.print("ppm");
    display.setCursor(5, 25);
    display.print("Esik: ");
    display.print(ayar.gazEsik);
    display.print("ppm");
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarHareketCiz() {
  if(ayarModu) {
    display.setCursor(5, 15);
    if(ayarSecim == 0) {
      display.print("Esik Sure:");
      display.setTextSize(2);
      display.setCursor(20, 30);
      display.print(ayar.hareketEsikSure/1000.0, 1);
      display.print(" sn");
    } else if(ayarSecim == 1) {
      display.print("Hassasiyet:");
      display.setTextSize(2);
      display.setCursor(20, 30);
      switch(ayar.guvenlikHassasiyet) {
        case 1: display.print("DUSUK"); break;
        case 2: display.print("ORTA"); break;
        case 3: display.print("YUKSEK"); break;
      }
    } else {
      display.print("Alarm Sure:");
      display.setTextSize(2);
      display.setCursor(20, 30);
      display.print(ayar.alarmSuresi/1000);
      display.print(" sn");
    }
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Esik Sure: ");
    display.print(ayar.hareketEsikSure/1000.0, 1);
    display.print(" sn");
    display.setCursor(5, 25);
    display.print("Hassasiyet: ");
    switch(ayar.guvenlikHassasiyet) {
      case 1: display.print("DUSUK"); break;
      case 2: display.print("ORTA"); break;
      case 3: display.print("YUKSEK"); break;
    }
    display.setCursor(5, 35);
    display.print("Alarm Sure: ");
    display.print(ayar.alarmSuresi/1000);
    display.print(" sn");
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarSesCiz() {
  display.setCursor(10, 20);
  display.print("Ses Durumu:");
  display.setTextSize(2);
  display.setCursor(40, 35);
  display.print(ayar.ses ? "ACIK" : "KAPALI");
  display.setTextSize(1);
  display.setCursor(10, 58);
  display.print("Buton: Degistir");
}

void ayarServoCiz() {
  if(ayarModu) {
    display.setCursor(5, 15);
    if(ayarSecim == 0) {
      display.print("Tarama Hizi:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoSpeed);
      display.print("ms");
    } else if(ayarSecim == 1) {
      display.print("Min Aci:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoMinAci);
      display.print("°");
    } else if(ayarSecim == 2) {
      display.print("Max Aci:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoMaxAci);
      display.print("°");
    } else {
      display.print("Joy Hizi:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.joyServoHiz);
      display.print("ms");
    }
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Tarama Hiz: ");
    display.print(ayar.servoSpeed);
    display.print("ms");
    display.setCursor(5, 25);
    display.print("Min: ");
    display.print(ayar.servoMinAci);
    display.print("°");
    display.setCursor(5, 35);
    display.print("Max: ");
    display.print(ayar.servoMaxAci);
    display.print("°");
    display.setCursor(5, 45);
    display.print("Joy Hiz: ");
    display.print(ayar.joyServoHiz);
    display.print("ms");
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarLedCiz() {
  if(ayarModu) {
    display.setCursor(5, 15);
    if(ayarSecim == 0) {
      display.print("LED Modu:");
      display.setTextSize(2);
      display.setCursor(15, 30);
      switch(ayar.ledMode) {
        case 0: display.print("SABIT"); break;
        case 1: display.print("Y.SON"); break;
        case 2: display.print("MESAFE"); break;
        case 3: display.print("HAREKET"); break;
      }
    } else if(ayarSecim == 1) {
      display.print("LED Hizi:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.ledHiz);
      display.print("ms");
    } else {
      display.print("Parlaklik:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.ledBright);
    }
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Mod: ");
    switch(ayar.ledMode) {
      case 0: display.print("SABIT"); break;
      case 1: display.print("YANIP SONME"); break;
      case 2: display.print("MESAFE"); break;
      case 3: display.print("HAREKET"); break;
    }
    display.setCursor(5, 25);
    display.print("Hiz: ");
    display.print(ayar.ledHiz);
    display.print("ms");
    display.setCursor(5, 35);
    display.print("Parlaklik: ");
    display.print(ayar.ledBright);
    display.setCursor(5, 45);
    display.print("Durum: ");
    display.print(ayar.ledAktif ? "AKTIF" : "PASIF");
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void sistemEkranCiz() {
  display.setCursor(5, 10);
  display.print("TOPRAK PRO MAX V5.0");
  display.setCursor(5, 20);
  display.print("Hafiza: ");
  display.print(ESP.getFreeHeap() / 1024.0, 1);
  display.print("KB");
  display.setCursor(5, 30);
  display.print("Servo: ");
  display.print(servoAci);
  display.print("°");
  display.setCursor(5, 40);
  display.print("Mesafe: ");
  display.print(mesafe, 1);
  display.print("cm");
  display.setCursor(5, 50);
  display.print("LED Parlaklik: ");
  display.print(ledParlaklik);
  display.setCursor(5, 60);
  display.print("D18: Ana Menu");
}

void guvenlikEkranCiz() {
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print(guvenlikAktif ? "AKTIF" : "PASIF");
  
  display.setTextSize(1);
  display.setCursor(5, 45);
  if(guvenlikAktif) {
    display.print("Durum: ");
    if(alarmCalisiyor) {
      display.print("ALARM!");
      int kalanSure = (ayar.alarmSuresi - (millis() - alarmBaslangic)) / 1000;
      display.setCursor(5, 55);
      display.print("Kalan: ");
      display.print(kalanSure);
      display.print(" sn");
    } else {
      display.print("BEKLIYOR");
    }
  } else {
    display.print("Guvenlik sistemi");
    display.setCursor(5, 55);
    display.print("pasif durumda");
  }
}

void otonomEkranCiz() {
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print(otonomMod ? "AKTIF" : "PASIF");
  
  display.setTextSize(1);
  if(otonomMod) {
    display.setCursor(5, 45);
    display.print("Durum: ");
    switch(otonomDurum) {
      case 0: display.print("DUR"); break;
      case 1: display.print("ILERI"); break;
      case 2: display.print("SAG"); break;
      case 3: display.print("SOL"); break;
      case 4: display.print("GERI"); break;
    }
    display.setCursor(5, 55);
    display.print("Mesafe: ");
    display.print(mesafe);
    display.print(" cm");
  } else {
    display.setCursor(5, 45);
    display.print("Otonom mod");
    display.setCursor(5, 55);
    display.print("pasif durumda");
  }
}

void ayarPirCiz() {
  display.setCursor(10, 15);
  display.print("PIR Kalibrasyon");
  
  if(pirKalibrasyonYapiliyor) {
    int gecenSure = (millis() - pirKalibrasyonBaslangic) / 1000;
    int kalanSure = 5 - gecenSure;
    
    display.setCursor(10, 30);
    display.print("Calisiyor...");
    display.setCursor(10, 40);
    display.print("Kalan: ");
    display.print(kalanSure);
    display.print(" saniye");
    
    int ilerleme = map(gecenSure, 0, 5, 0, 100);
    display.drawRect(10, 50, 100, 8, SSD1306_WHITE);
    display.fillRect(10, 50, ilerleme, 8, SSD1306_WHITE);
  } else {
    display.setCursor(10, 30);
    display.print("Hazir");
    display.setCursor(10, 40);
    display.print("Hassasiyet: ");
    switch(ayar.pirHassasiyet) {
      case 1: display.print("Cok Dusuk"); break;
      case 2: display.print("Dusuk"); break;
      case 3: display.print("Orta"); break;
    }
    display.setCursor(10, 50);
    display.print("Filtre: ");
    display.print(ayar.pirFiltre ? "AKTIF" : "PASIF");
    display.setCursor(10, 60);
    display.print("Buton: Baslat");
  }
}

void robotCiz(int x, int y) {
  display.drawCircle(x, y, 6, SSD1306_WHITE);
  
  if(gozAcik) {
    if(robotIfade == 0) {
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawPixel(x, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 5) {
      display.fillCircle(x-3, y-2, 2, SSD1306_WHITE);
      display.fillCircle(x+3, y-2, 2, SSD1306_WHITE);
      display.drawLine(x-3, y+2, x+3, y+2, SSD1306_WHITE);
    }
  } else {
    display.drawLine(x-3, y-1, x-1, y-1, SSD1306_WHITE);
    display.drawLine(x+1, y-1, x+3, y-1, SSD1306_WHITE);
  }
  
  if(uyariVar) {
    display.drawTriangle(x+8, y-8, x+8, y-3, x+5, y-5, SSD1306_WHITE);
  }
}

void altBilgiCiz() {
  display.drawFastHLine(0, 63, 128, SSD1306_WHITE);
  
  if(uyariVar) {
    display.setCursor(5, 56);
    display.print("UYARI!");
  } else {
    display.setCursor(5, 56);
    display.print("D18:Ana Menu");
    display.setCursor(85, 56);
    display.print("JOY:Sec");
  }
}

// =========== SES FONKSİYONLARI ===========
void bipSesi(int tip) {
  if(!ayar.ses) return;
  
  switch(tip) {
    case 1:
      tone(BUZZER_PIN, 800, 50);
      break;
    case 2:
      tone(BUZZER_PIN, 1000, 100);
      break;
    case 3:
      tone(BUZZER_PIN, 400, 200);
      break;
  }
}
