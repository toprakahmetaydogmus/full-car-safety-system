/****************************************************
 * TOPRAK - Akıllı Araba Asistanı PRO MAX
 * GELİŞTİRİLMİŞ SON SÜRÜM V4.0
 * 
 * SENSÖRLER:
 * - HC-SR04 Ultrasonik Mesafe Sensörü
 * - MQ-2 Gaz Sensörü
 * - HC-SR501 PIR Hareket Sensörü
 * - SG90 Servo Motor
 * - 0.96" OLED Ekran
 * - Buzzer
 * - RGB LED
 * 
 * KONTROL:
 * D18 BUTONU: Ana menüye dön
 * JOYSTICK BUTONU: Seç / Açar/Kapat
 * JOYSTICK YUKARI/AŞAĞI: Menü gezin
 * JOYSTICK SAĞ/SOL: Değer değiştir
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
#define GERI_BUTON 18  // D18 - Ana menü butonu

#define TRIG_PIN 13
#define ECHO_PIN 12
#define GAS_PIN 36
#define PIR_PIN 14     // HCSR501 PIR sensörü
#define BUZZER_PIN 15
#define SYS_LED 2
#define SERVO_PIN 26
#define LED_PIN 25
#define LED_R 33       // RGB LED Kırmızı
#define LED_G 27       // RGB LED Yeşil
#define LED_B 32       // RGB LED Mavi

// =========== NESNELER ===========
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo taramaServo;

// =========== DEĞİŞKENLER ===========
// Sensörler
float mesafe = 0;
float gaz = 0;
bool hareket = false;
unsigned long sonHareketZamani = 0;
int hareketSayisi = 0;

// Butonlar
bool joyBtn = false, lastJoyBtn = false;
bool geriBtn = false, lastGeriBtn = false;

// Menü Sistemi
#define MENU_ANA 0
#define MENU_MESAFE 1
#define MENU_GAZ 2
#define MENU_HAREKET 3
#define MENU_TARAMA 4
#define MENU_AYAR_MESAFE 5
#define MENU_AYAR_GAZ 6
#define MENU_AYAR_HAREKET 7
#define MENU_AYAR_SES 8
#define MENU_AYAR_SERVO 9
#define MENU_AYAR_LED 10
#define MENU_SISTEM 11
#define MENU_GUVENLIK 12
#define MENU_OTONOM 13

int aktifMenu = MENU_ANA;
int menuSecim = 0;
bool ayarModu = false;
int ayarSecim = 0;

// Ana Menü Kaydırma
int menuBaslangic = 0;
const int MAX_GORUNEN_OGE = 6;

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0; // 0: yok, 1: dikkat, 2: tehlike

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0; // 0: normal, 1: dikkat, 2: tehlike, 3: mutlu, 4: uyku, 5: hareket

// Servo Sistemi
int servoAci = 90;
int hedefAci = 90;
bool taramaYonu = true;
bool taramaAktif = true;
int taramaHizi = 20;
float taramaHaritasi[36] = {0};
unsigned long sonServoHareket = 0;
bool alarmModu = false;
int alarmServoHizi = 50;

// LED Sistemi
int ledParlaklik = 150;
int ledMod = 0; // 0: Sabit, 1: Yanıp Sönme, 2: Mesafe Feedback, 3: Hareket Feedback
bool ledDurum = true;
unsigned long sonLedDegisim = 0;
int ledR = 0, ledG = 0, ledB = 0;

// Güvenlik Sistemi
bool guvenlikAktif = false;
bool alarmCalisiyor = false;
unsigned long alarmBaslangic = 0;
int alarmSeviye = 0;

// Hareket Analizi
bool hareketTespitEdildi = false;
unsigned long hareketSure = 0;
int hareketSikligi = 0;
unsigned long sonHareketAnaliz = 0;

// Otonom Mod
bool otonomMod = false;
int otonomDurum = 0; // 0: DUR, 1: ILERI, 2: SAG, 3: SOL, 4: GERI
unsigned long sonOtonomKarar = 0;

// Ayarlar
struct {
  int mesafeDikkat = 50;   // cm
  int mesafeTehlike = 20;  // cm
  int gazEsik = 800;       // ppm
  int hareketEsikSure = 3000; // ms
  bool ses = true;
  
  // Servo ayarları
  bool servoAktif = true;
  int servoSpeed = 20;
  int servoMinAci = 20;
  int servoMaxAci = 160;
  bool servoAlarmMod = false;
  
  // LED ayarları
  bool ledAktif = true;
  int ledMode = 0;
  int ledHiz = 500;
  int ledBright = 150;
  
  // Güvenlik ayarları
  bool guvenlikOtomatik = false;
  int guvenlikHassasiyet = 2; // 1: Düşük, 2: Orta, 3: Yüksek
  int alarmSuresi = 10000; // ms
  
  // Robot animasyon
  bool robotAnimasyon = true;
  
  // Otonom mod
  bool otonomAktif = false;
  int otonomHiz = 30;
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
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  
  // Servo
  taramaServo.attach(SERVO_PIN);
  taramaServo.write(servoAci);
  delay(300);
  
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
  Serial.println("TOPRAK PRO MAX V4.0 - Sistem hazir!");
}

void baslangic() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPRAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("PRO MAX V4.0");
  display.display();
  
  // RGB LED test
  rgbLED(255, 0, 0);   // Kırmızı
  delay(300);
  rgbLED(0, 255, 0);   // Yeşil
  delay(300);
  rgbLED(0, 0, 255);   // Mavi
  delay(300);
  rgbLED(255, 255, 0); // Sarı
  delay(300);
  rgbLED(0, 0, 0);     // Kapalı
  
  // LED test
  if(ayar.ledAktif) {
    for(int i = 0; i <= 255; i += 5) {
      analogWrite(LED_PIN, i);
      delay(10);
    }
    for(int i = 255; i >= 0; i -= 5) {
      analogWrite(LED_PIN, i);
      delay(10);
    }
    analogWrite(LED_PIN, ledParlaklik);
  }
  
  // Servo test
  if(ayar.servoAktif) {
    taramaServo.write(0);
    delay(400);
    taramaServo.write(180);
    delay(400);
    taramaServo.write(90);
    delay(400);
  }
  
  // Ses test
  if(ayar.ses) {
    tone(BUZZER_PIN, 523, 200);
    delay(250);
    tone(BUZZER_PIN, 659, 200);
    delay(250);
    tone(BUZZER_PIN, 784, 300);
  }
  
  // Sistem LED test
  for(int i=0; i<3; i++) {
    digitalWrite(SYS_LED, HIGH);
    delay(200);
    digitalWrite(SYS_LED, LOW);
    delay(200);
  }
  
  delay(1500);
}

// =========== RGB LED KONTROLÜ ===========
void rgbLED(int r, int g, int b) {
  analogWrite(LED_R, 255 - r); // Ortak anot
  analogWrite(LED_G, 255 - g);
  analogWrite(LED_B, 255 - b);
}

// =========== ANA DÖNGÜ ===========
void loop() {
  unsigned long simdi = millis();
  
  // 1. Sensör okuma (200ms)
  if(simdi - sonOkuma > 200) {
    sensorOku();
    uyariKontrol();
    sonOkuma = simdi;
  }
  
  // 2. Buton kontrolü
  butonOku();
  butonIsle();
  
  // 3. Joystick kontrolü
  joyKontrol();
  
  // 4. Servo kontrolü
  if(ayar.servoAktif) {
    if(alarmModu && alarmCalisiyor) {
      alarmServoKontrol(simdi);
    } else if(taramaAktif && simdi - sonServoHareket > taramaHizi) {
      servoKontrol();
      sonServoHareket = simdi;
    }
  }
  
  // 5. LED kontrolü
  if(ayar.ledAktif && simdi - sonLedDegisim > ayar.ledHiz) {
    ledKontrol(simdi);
    sonLedDegisim = simdi;
  }
  
  // 6. Güvenlik sistemi
  guvenlikKontrol(simdi);
  
  // 7. Otonom mod
  if(otonomMod) {
    otonomKontrol(simdi);
  }
  
  // 8. Ekran güncelle
  ekranCiz();
  
  // 9. Ses ve LED uyarıları
  sesLED();
  
  // 10. Animasyon
  if(ayar.robotAnimasyon && simdi - sonGozKirpma > 3000 + random(0, 2000)) {
    gozAcik = !gozAcik;
    sonGozKirpma = simdi;
  }
  
  // 11. Uyku modu kontrolü
  if(simdi - sonEtkilesim > 20000 && !uykuModu && !guvenlikAktif) {
    uykuModu = true;
    robotIfade = 4;
  } else if(simdi - sonEtkilesim < 20000 && uykuModu) {
    uykuModu = false;
    robotIfade = 0;
  }
  
  delay(50);
}

// =========== SENSÖR FONKSİYONLARI ===========
void sensorOku() {
  // Mesafe
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long sure = pulseIn(ECHO_PIN, HIGH, 30000);
  mesafe = sure * 0.034 / 2;
  if(mesafe <= 0 || mesafe > 400) mesafe = 400;
  
  // Gaz
  int gazDeger = analogRead(GAS_PIN);
  gaz = map(gazDeger, 0, 4095, 0, 2000);
  
  // PIR Hareket sensörü
  bool yeniHareket = (digitalRead(PIR_PIN) == HIGH);
  
  if(yeniHareket && !hareket) {
    hareket = true;
    hareketTespitEdildi = true;
    sonHareketZamani = millis();
    hareketSayisi++;
    robotIfade = 5; // Hareket ifadesi
    
    if(guvenlikAktif && ayar.guvenlikOtomatik) {
      alarmSeviye = 1;
      alarmCalisiyor = true;
      alarmBaslangic = millis();
    }
  } else if(!yeniHareket && hareket) {
    hareket = false;
    hareketSure = millis() - sonHareketZamani;
  }
  
  // Hareket analizi (10 saniyede bir)
  if(millis() - sonHareketAnaliz > 10000) {
    hareketSikligi = hareketSayisi;
    hareketSayisi = 0;
    sonHareketAnaliz = millis();
  }
  
  // Debug
  static unsigned long sonSerial = 0;
  if(millis() - sonSerial > 1000) {
    Serial.print("M:");
    Serial.print(mesafe);
    Serial.print("cm G:");
    Serial.print(gaz);
    Serial.print("ppm H:");
    Serial.print(hareket ? "EVET" : "HAYIR");
    Serial.print(" Sayi:");
    Serial.println(hareketSikligi);
    sonSerial = millis();
  }
}

void uyariKontrol() {
  bool oncekiUyari = uyariVar;
  int oncekiSeviye = uyariSeviye;
  
  uyariVar = false;
  uyariSeviye = 0;
  
  if(mesafe < ayar.mesafeTehlike) {
    uyariVar = true;
    uyariSeviye = 2;
    robotIfade = 2;
  } else if(mesafe < ayar.mesafeDikkat) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
  } else if(gaz > ayar.gazEsik) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
  } else if(hareketTespitEdildi && guvenlikAktif) {
    uyariVar = true;
    uyariSeviye = alarmSeviye;
    robotIfade = 5;
  } else {
    robotIfade = 0;
  }
  
  // Uyku modundaysa
  if(uykuModu) {
    robotIfade = 4;
  }
  
  // Hareket tespiti 3 saniye sonra sıfırlanır
  if(hareketTespitEdildi && millis() - sonHareketZamani > 3000) {
    hareketTespitEdildi = false;
  }
  
  if((oncekiUyari != uyariVar) || (oncekiSeviye != uyariSeviye)) {
    if(!uyariVar && ayar.ses) {
      noTone(BUZZER_PIN);
      sonBip = 0;
    }
  }
  
  digitalWrite(SYS_LED, uyariVar);
}

// =========== SERVO KONTROLÜ ===========
void servoKontrol() {
  if(servoAci != hedefAci) {
    if(servoAci < hedefAci) servoAci++;
    else servoAci--;
    taramaServo.write(servoAci);
  } else {
    if(taramaYonu) {
      hedefAci += 5;
      if(hedefAci > ayar.servoMaxAci) {
        hedefAci = ayar.servoMaxAci;
        taramaYonu = false;
      }
    } else {
      hedefAci -= 5;
      if(hedefAci < ayar.servoMinAci) {
        hedefAci = ayar.servoMinAci;
        taramaYonu = true;
      }
    }
    
    int taramaIndex = map(servoAci, 0, 180, 0, 35);
    taramaHaritasi[taramaIndex] = mesafe;
  }
}

void alarmServoKontrol(unsigned long simdi) {
  static unsigned long sonAlarmHareket = 0;
  static bool alarmYonu = true;
  
  if(simdi - sonAlarmHareket > alarmServoHizi) {
    if(alarmYonu) {
      servoAci += 10;
      if(servoAci > 160) {
        servoAci = 160;
        alarmYonu = false;
      }
    } else {
      servoAci -= 10;
      if(servoAci < 20) {
        servoAci = 20;
        alarmYonu = true;
      }
    }
    taramaServo.write(servoAci);
    sonAlarmHareket = simdi;
  }
}

// =========== LED KONTROLÜ ===========
void ledKontrol(unsigned long simdi) {
  switch(ledMod) {
    case 0: // Sabit beyaz
      analogWrite(LED_PIN, ledParlaklik);
      rgbLED(0, 0, 0);
      break;
      
    case 1: // Yanıp sönme (beyaz)
      ledDurum = !ledDurum;
      analogWrite(LED_PIN, ledDurum ? ledParlaklik : 0);
      rgbLED(0, 0, 0);
      break;
      
    case 2: // Mesafe feedback (RGB)
      analogWrite(LED_PIN, 0);
      if(mesafe < 30) {
        rgbLED(255, 0, 0); // Kırmızı
      } else if(mesafe < 60) {
        rgbLED(255, 255, 0); // Sarı
      } else if(mesafe < 100) {
        rgbLED(0, 255, 0); // Yeşil
      } else {
        rgbLED(0, 0, 255); // Mavi
      }
      break;
      
    case 3: // Hareket feedback
      analogWrite(LED_PIN, 0);
      if(hareketTespitEdildi) {
        rgbLED(255, 165, 0); // Turuncu
      } else {
        rgbLED(0, 0, 0); // Kapalı
      }
      break;
  }
}

// =========== GÜVENLİK SİSTEMİ ===========
void guvenlikKontrol(unsigned long simdi) {
  if(!guvenlikAktif) {
    alarmCalisiyor = false;
    alarmModu = false;
    return;
  }
  
  if(alarmCalisiyor) {
    // Alarm süresi kontrolü
    if(simdi - alarmBaslangic > ayar.alarmSuresi) {
      alarmCalisiyor = false;
      alarmModu = false;
      noTone(BUZZER_PIN);
      if(ayar.servoAktif) {
        taramaServo.write(90);
        servoAci = 90;
      }
    } else {
      // Alarm devam ediyor
      alarmModu = true;
      
      // RGB LED kırmızı yanıp sönsün
      if((simdi / 200) % 2 == 0) {
        rgbLED(255, 0, 0);
      } else {
        rgbLED(0, 0, 0);
      }
    }
  }
  
  // Hareket hassasiyetine göre alarm seviyesi
  if(hareketTespitEdildi && !alarmCalisiyor) {
    if(ayar.guvenlikHassasiyet == 1) {
      // Düşük hassasiyet - sadece LED
      rgbLED(255, 255, 0);
    } else if(ayar.guvenlikHassasiyet == 2) {
      // Orta hassasiyet - kısa bip
      alarmSeviye = 1;
      if(ayar.ses) tone(BUZZER_PIN, 800, 500);
    } else {
      // Yüksek hassasiyet - tam alarm
      alarmSeviye = 2;
      alarmCalisiyor = true;
      alarmBaslangic = simdi;
    }
  }
}

// =========== OTONOM MOD ===========
void otonomKontrol(unsigned long simdi) {
  if(simdi - sonOtonomKarar > 1000) { // Her saniye karar ver
    sonOtonomKarar = simdi;
    
    // Mesafeye göre karar ver
    if(mesafe < ayar.mesafeTehlike) {
      otonomDurum = 4; // GERİ
      taramaServo.write(90); // Düz bak
      servoAci = 90;
    } else if(mesafe < ayar.mesafeDikkat) {
      // Engel var, yön ara
      taramaServo.write(0);
      delay(300);
      float solMesafe = mesafe;
      
      taramaServo.write(180);
      delay(300);
      float sagMesafe = mesafe;
      
      taramaServo.write(90);
      delay(300);
      
      if(solMesafe > sagMesafe && solMesafe > ayar.mesafeDikkat) {
        otonomDurum = 3; // SOL
      } else if(sagMesafe > ayar.mesafeDikkat) {
        otonomDurum = 2; // SAĞ
      } else {
        otonomDurum = 4; // GERİ
      }
    } else {
      otonomDurum = 1; // İLERİ
    }
    
    // Otonom duruma göre LED rengi
    switch(otonomDurum) {
      case 1: rgbLED(0, 255, 0); break; // İleri - Yeşil
      case 2: rgbLED(255, 255, 0); break; // Sağ - Sarı
      case 3: rgbLED(255, 165, 0); break; // Sol - Turuncu
      case 4: rgbLED(255, 0, 0); break; // Geri - Kırmızı
    }
  }
}

// =========== BUTON KONTROLÜ ===========
void butonOku() {
  joyBtn = (digitalRead(JOY_BTN) == LOW);
  geriBtn = (digitalRead(GERI_BUTON) == LOW);
}

void butonIsle() {
  static unsigned long sonTiklama = 0;
  unsigned long simdi = millis();
  
  if(simdi - sonTiklama < 300) return;
  
  // D18 BUTONU - HER ZAMAN ANA MENÜ
  if(geriBtn && !lastGeriBtn) {
    aktifMenu = MENU_ANA;
    ayarModu = false;
    menuSecim = 0;
    menuBaslangic = 0;
    ayarSecim = 0;
    robotIfade = 3;
    bipSesi(2);
    sonEtkilesim = simdi;
    uykuModu = false;
    sonTiklama = simdi;
  }
  
  // JOYSTICK BUTONU
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
          case 4: aktifMenu = MENU_AYAR_MESAFE; break;
          case 5: aktifMenu = MENU_AYAR_GAZ; break;
          case 6: aktifMenu = MENU_AYAR_HAREKET; break;
          case 7: aktifMenu = MENU_AYAR_SES; break;
          case 8: aktifMenu = MENU_AYAR_SERVO; break;
          case 9: aktifMenu = MENU_AYAR_LED; break;
          case 10: aktifMenu = MENU_SISTEM; break;
          case 11: aktifMenu = MENU_GUVENLIK; break;
          case 12: aktifMenu = MENU_OTONOM; break;
        }
        break;
        
      case MENU_TARAMA:
        taramaAktif = !taramaAktif;
        break;
        
      case MENU_GUVENLIK:
        guvenlikAktif = !guvenlikAktif;
        if(guvenlikAktif) {
          bipSesi(3); // Alarm sesi
        } else {
          alarmCalisiyor = false;
          alarmModu = false;
          noTone(BUZZER_PIN);
        }
        break;
        
      case MENU_OTONOM:
        otonomMod = !otonomMod;
        if(!otonomMod) {
          rgbLED(0, 0, 0);
          otonomDurum = 0;
        }
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
  
  if(joyY < 1000) { // YUKARI
    hareketVar = true;
    if(aktifMenu == MENU_ANA) {
      menuSecim--;
      if(menuSecim < 0) menuSecim = 12;
      if(menuSecim < menuBaslangic) {
        menuBaslangic = menuSecim;
      } else if(menuSecim >= menuBaslangic + MAX_GORUNEN_OGE) {
        menuBaslangic = menuSecim - MAX_GORUNEN_OGE + 1;
      }
    } else if(ayarModu) {
      ayarSecim--;
    }
  }
  else if(joyY > 3000) { // AŞAĞI
    hareketVar = true;
    if(aktifMenu == MENU_ANA) {
      menuSecim++;
      if(menuSecim > 12) menuSecim = 0;
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
        ayar.servoSpeed += yon * 5;
        ayar.servoSpeed = constrain(ayar.servoSpeed, 5, 100);
        taramaHizi = ayar.servoSpeed;
      } else if(ayarSecim == 1) {
        ayar.servoMinAci += yon * 5;
        ayar.servoMinAci = constrain(ayar.servoMinAci, 0, 170);
      } else {
        ayar.servoMaxAci += yon * 5;
        ayar.servoMaxAci = constrain(ayar.servoMaxAci, 10, 180);
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
    case MENU_AYAR_MESAFE: display.print("AYAR MESAFE"); break;
    case MENU_AYAR_GAZ: display.print("AYAR GAZ"); break;
    case MENU_AYAR_HAREKET: display.print("AYAR HAREKET"); break;
    case MENU_AYAR_SES: display.print("AYAR SES"); break;
    case MENU_AYAR_SERVO: display.print("AYAR SERVO"); break;
    case MENU_AYAR_LED: display.print("AYAR LED"); break;
    case MENU_SISTEM: display.print("SISTEM"); break;
    case MENU_GUVENLIK: display.print("GUVENLIK"); break;
    case MENU_OTONOM: display.print("OTONOM MOD"); break;
  }
}

void icerikCiz() {
  switch(aktifMenu) {
    case MENU_ANA: anaMenuCiz(); break;
    case MENU_MESAFE: mesafeEkranCiz(); break;
    case MENU_GAZ: gazEkranCiz(); break;
    case MENU_HAREKET: hareketEkranCiz(); break;
    case MENU_TARAMA: taramaEkranCiz(); break;
    case MENU_AYAR_MESAFE: ayarMesafeCiz(); break;
    case MENU_AYAR_GAZ: ayarGazCiz(); break;
    case MENU_AYAR_HAREKET: ayarHareketCiz(); break;
    case MENU_AYAR_SES: ayarSesCiz(); break;
    case MENU_AYAR_SERVO: ayarServoCiz(); break;
    case MENU_AYAR_LED: ayarLedCiz(); break;
    case MENU_SISTEM: sistemEkranCiz(); break;
    case MENU_GUVENLIK: guvenlikEkranCiz(); break;
    case MENU_OTONOM: otonomEkranCiz(); break;
  }
}

void anaMenuCiz() {
  const char* menuItems[] = {
    "Mesafe Goster",
    "Gaz Goster",
    "Hareket Goster",
    "Tarama Ekrani",
    "Mesafe Ayari",
    "Gaz Ayari",
    "Hareket Ayari",
    "Ses Ayari",
    "Servo Ayari",
    "LED Ayari",
    "Sistem Bilgisi",
    "Guvenlik Sistemi",
    "Otonom Mod"
  };
  
  int baslangic = menuBaslangic;
  int bitis = min(baslangic + MAX_GORUNEN_OGE, 13);
  
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
  
  if(13 > MAX_GORUNEN_OGE) {
    int scrollBarHeight = 40;
    int scrollBarY = 15;
    int scrollPos = map(menuBaslangic, 0, 13 - MAX_GORUNEN_OGE, scrollBarY, scrollBarY + scrollBarHeight - 5);
    display.drawRect(125, scrollBarY, 2, scrollBarHeight, SSD1306_WHITE);
    display.fillRect(125, scrollPos, 2, 5, SSD1306_WHITE);
  }
}

void mesafeEkranCiz() {
  display.setTextSize(3);
  display.setCursor(15, 15);
  if(mesafe >= 100) display.print(">100");
  else if(mesafe >= 10) display.print(mesafe, 0);
  else display.print(mesafe, 1);
  
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("cm");
  
  display.drawRect(10, 50, 100, 8, SSD1306_WHITE);
  int barGenislik = map(constrain(mesafe, 0, 100), 0, 100, 0, 100);
  
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
  display.print("Son Tespit: ");
  if(hareketTespitEdildi) {
    int gecenSure = (millis() - sonHareketZamani) / 1000;
    display.print(gecenSure);
    display.print(" sn once");
  } else {
    display.print("---");
  }
  
  display.setCursor(5, 55);
  display.print("Siklik: ");
  display.print(hareketSikligi);
  display.print("/10 sn");
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
  
  for(int i = 0; i < 36; i++) {
    if(taramaHaritasi[i] > 2 && taramaHaritasi[i] < 100) {
      float aci = radians(i * 10);
      float uzaklik = map(taramaHaritasi[i], 0, 100, 0, yaricap);
      int x = merkezX + sin(aci) * uzaklik;
      int y = merkezY - cos(aci) * uzaklik;
      display.fillCircle(x, y, 1, SSD1306_WHITE);
    }
  }
  
  display.setCursor(5, 55);
  display.print("A:");
  display.print(servoAci);
  display.print(" ");
  display.print(taramaAktif ? "AKTIF" : "DURDU");
  display.print(alarmModu ? " ALARM" : "");
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
      display.print("Servo Hizi:");
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
    } else {
      display.print("Max Aci:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoMaxAci);
      display.print("°");
    }
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Hiz: ");
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
    display.print("Durum: ");
    display.print(ayar.servoAktif ? "AKTIF" : "PASIF");
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
  display.print("TOPRAK PRO MAX V4.0");
  display.setCursor(5, 20);
  display.print("Hafiza: ");
  display.print(ESP.getFreeHeap() / 1024.0, 1);
  display.print("KB");
  display.setCursor(5, 30);
  display.print("Uyku: ");
  display.print(uykuModu ? "AKTIF" : "PASIF");
  display.setCursor(5, 40);
  display.print("Alarm: ");
  display.print(alarmCalisiyor ? "CALISIYOR" : "PASIF");
  display.setCursor(5, 50);
  display.print("Hareket: ");
  display.print(hareketSikligi);
  display.print("/10 sn");
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

void robotCiz(int x, int y) {
  display.drawCircle(x, y, 6, SSD1306_WHITE);
  
  if(gozAcik) {
    if(robotIfade == 0) {
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawPixel(x, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 1) {
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawLine(x-2, y+2, x+2, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 2) {
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawLine(x-2, y+2, x+2, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 3) {
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawLine(x-2, y+3, x+2, y+3, SSD1306_WHITE);
      display.drawPixel(x-1, y+2, SSD1306_WHITE);
      display.drawPixel(x+1, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 4) {
      display.drawLine(x-3, y-1, x-1, y-1, SSD1306_WHITE);
      display.drawLine(x+1, y-1, x+3, y-1, SSD1306_WHITE);
      display.drawPixel(x, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 5) {
      display.fillCircle(x-3, y-1, 2, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 2, SSD1306_WHITE);
      display.drawLine(x-3, y+2, x+3, y+2, SSD1306_WHITE);
    }
  } else {
    display.drawLine(x-3, y-1, x-1, y-1, SSD1306_WHITE);
    display.drawLine(x+1, y-1, x+3, y-1, SSD1306_WHITE);
  }
  
  if(uyariVar) {
    display.drawTriangle(x+8, y-8, x+8, y-3, x+5, y-5, SSD1306_WHITE);
    display.drawPixel(x+7, y-5, SSD1306_WHITE);
  }
}

void altBilgiCiz() {
  display.drawFastHLine(0, 63, 128, SSD1306_WHITE);
  
  if(uyariVar) {
    display.setCursor(5, 56);
    display.print("UYARI!");
    if(uyariSeviye == 1) display.print(" (Dikkat)");
    else if(uyariSeviye == 2) display.print(" (Tehlike)");
  } else {
    display.setCursor(5, 56);
    display.print("D18: Ana Menu");
    display.setCursor(85, 56);
    display.print("JOY: Sec");
  }
}

// =========== SES FONKSİYONLARI ===========
void bipSesi(int tip) {
  if(!ayar.ses) return;
  
  switch(tip) {
    case 1: // Kıklama
      tone(BUZZER_PIN, 800, 50);
      break;
    case 2: // Onay
      tone(BUZZER_PIN, 1000, 100);
      break;
    case 3: // Hata/Alarm
      tone(BUZZER_PIN, 400, 200);
      break;
  }
}

void sesLED() {
  if(!uyariVar || !ayar.ses) return;
  
  unsigned long simdi = millis();
  int bipAraligi;
  
  if(uyariSeviye == 1) {
    bipAraligi = 1000;
  } else {
    bipAraligi = 300;
  }
  
  if(simdi - sonBip > bipAraligi) {
    tone(BUZZER_PIN, uyariSeviye == 1 ? 600 : 800, 100);
    sonBip = simdi;
  }
}
