/****************************************************
 * TOPROAK PRO MAX V7.2 - ROBOTİK ASİSTAN
 * ESP32 UYUMLU - TÜM SENSÖRLER ÇALIŞIYOR
 * GÜNCELLENMİŞ TAM KOD
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
#define LED_PIN 25

// =========== NESNELER ===========
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo taramaServo;

// =========== ESP32 PWM AYARLARI ===========
#define BUZZER_CHANNEL 0
#define LED_CHANNEL 1
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// =========== MENÜ TANIMLARI ===========
#define MENU_ANA 0
#define MENU_MESAFE 1
#define MENU_GAZ 2
#define MENU_HAREKET 3
#define MENU_TARAMA 4
#define MENU_SERVO_MANUEL 5
#define MENU_AYAR_MESAFE 6
#define MENU_AYAR_GAZ 7
#define MENU_AYAR_HAREKET 8
#define MENU_AYAR_SES 9
#define MENU_AYAR_SERVO 10
#define MENU_AYAR_LED 11
#define MENU_SISTEM 12
#define MENU_GUVENLIK 13
#define MENU_KALIBRASYON 14
#define MENU_ISTATISTIK 15
#define MENU_TEST 16
#define MENU_HABERLESME 17
#define MENU_GELISMIS 18
#define MENU_DIAGNOSTIC 19
#define MENU_FIRMWARE 20

// =========== GLOBAL DEĞİŞKENLER ===========
// Sensörler
float mesafe = 0;
float gaz = 0;
bool hareket = false;

// HC-SR04
#define MESAFE_BUFFER_SIZE 5
float mesafeBuffer[MESAFE_BUFFER_SIZE];
int bufferIndex = 0;
unsigned long sonMesafeOkuma = 0;
const int MESAFE_OKUMA_ARALIGI = 100;
bool mesafeSensorCalisiyor = true;
int mesafeHataSayisi = 0;
float sonGecerliMesafe = 100.0;

// PIR
unsigned long sonPirOkuma = 0;
const int PIR_OKUMA_ARALIGI = 150;
unsigned long sonHareketZamani = 0;
int hareketSayisi = 0;
bool hareketTespitEdildi = false;

// Butonlar
bool joyBtn = false, lastJoyBtn = false;
bool geriBtn = false, lastGeriBtn = false;

// Menü Sistemi
int aktifMenu = MENU_ANA;
int menuSecim = 0;
bool ayarModu = false;
int ayarSecim = 0;
bool manuelServoMode = false;
int menuBaslangic = 0;
const int MAX_GORUNEN_OGE = 6;
const int TOPLAM_MENU_OGE = 21;

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0;
unsigned long sonUyariZamani = 0;
bool beepCalisiyor = false;

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0;

// =========== SERVO SİSTEMİ ===========
// SERVO SINIRLARI DEĞİŞTİRİLDİ: 0-150 derece
const int SERVO_MIN_LIMIT = 0;      // 30'dan 0'a değiştirildi
const int SERVO_MAX_LIMIT = 150;    // 150 olarak kaldı
const int SERVO_HOME_POS = 90;
int servoAci = SERVO_HOME_POS;
int hedefAci = SERVO_HOME_POS;
bool taramaYonu = false;
bool taramaAktif = true;
int taramaHizi = 15;

// TARAMA HARİTASI 16 ELEMENT OLARAK DEĞİŞTİRİLDİ (0-150 derece, 10 derece aralıklarla)
float taramaHaritasi[16] = {0}; // 0, 10, 20, ..., 150 -> 16 elements
unsigned long sonServoHareket = 0;

// LED Sistemi
int ledParlaklik = 200;
int ledMod = 2;
bool ledDurum = true;
unsigned long sonLedDegisim = 0;
int ledBlinkSpeed = 1000;

// LED Mod Tanımları
#define LED_MOD_SABIT 0
#define LED_MOD_BLINK 1
#define LED_MOD_MESAFE 2
#define LED_MOD_HAREKET 3
#define LED_MOD_RAINBOW 4

// Buzzer Sistemi
unsigned long sonBip = 0;
int bipInterval = 1500;
bool bipDurum = false;

// Güvenlik Sistemi
bool guvenlikAktif = false;
bool alarmCalisiyor = false;
unsigned long alarmBaslangic = 0;
int alarmSeviye = 0;

// Joystick Servo Kontrol
int joyServoSpeed = 20;
unsigned long sonJoyServoHareket = 0;

// =========== AYARLAR ===========
struct {
  // UYARI EŞİKLERİ DEĞİŞTİRİLDİ: 100 cm minimum
  int mesafeDikkat = 100;    // 50'den 100'e çıkarıldı
  int mesafeTehlike = 50;    // 20'den 50'ye çıkarıldı
  int gazEsik = 800;
  int hareketEsikSure = 3000;
  bool ses = true;
  bool servoAktif = true;
  int servoSpeed = 15;
  // SERVO SINIRLARI DEĞİŞTİRİLDİ
  int servoMinAci = SERVO_MIN_LIMIT;  // 0
  int servoMaxAci = SERVO_MAX_LIMIT;  // 150
  bool ledAktif = true;
  int ledMode = 2;
  int ledHiz = 500;
  int ledBright = 200;
  bool guvenlikOtomatik = false;
  int maxMesafe = 500;
} ayar;

// Zamanlayıcılar
unsigned long sonOkuma = 0;
unsigned long sonEtkilesim = 0;
bool uykuModu = false;

// İstatistikler
struct {
  unsigned long baslangicZamani = 0;
  unsigned long calismaSuresi = 0;
  int mesafeOkumaSayisi = 0;
  int hareketSayisi = 0;
  int servoHareketSayisi = 0;
  float maxMesafe = 0;
  float minMesafe = 1000;
  float ortalamaMesafe = 0;
  float toplamMesafe = 0;
} istatistik;

// Sistem Durumu
struct {
  float pilSeviyesi = 100.0;
  float isikSeviyesi = 0;
  float sicaklik = 25.0;
  int serbestBellek = 0;
  int hataSayisi = 0;
} sistem;

// =========== ESP32 PWM FONKSİYONLARI ===========
void toneESP(int pin, int frequency, int duration) {
  ledcWriteTone(BUZZER_CHANNEL, frequency);
  if (duration > 0) {
    delay(duration);
    noToneESP();
  }
}

void noToneESP() {
  ledcWriteTone(BUZZER_CHANNEL, 0);
  ledcWrite(BUZZER_CHANNEL, 0);
}

void analogWriteESP(int pin, int value) {
  ledcWrite(LED_CHANNEL, value);
}

// =========== GELİŞMİŞ MESAFE ÖLÇÜMÜ (5 metre) ===========
float hcSr04Oku() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) {
    mesafeHataSayisi++;
    if (mesafeHataSayisi > 10) {
      mesafeSensorCalisiyor = false;
    }
    return sonGecerliMesafe;
  }
  
  float distance = duration * 0.0343 / 2;
  
  // 5 metre sınırı (500cm)
  if (distance > ayar.maxMesafe) {
    distance = ayar.maxMesafe;
  }
  
  if (distance < 2.0 || distance > ayar.maxMesafe) {
    return sonGecerliMesafe;
  }
  
  mesafeHataSayisi = 0;
  mesafeSensorCalisiyor = true;
  sonGecerliMesafe = distance;
  
  return distance;
}

float hcSr04OkuFiltreli() {
  float toplam = 0;
  int gecerliOkuma = 0;
  
  for (int i = 0; i < 3; i++) {
    float okuma = hcSr04Oku();
    
    if (okuma >= 2.0 && okuma <= ayar.maxMesafe) {
      toplam += okuma;
      gecerliOkuma++;
    }
    delay(5);
  }
  
  if (gecerliOkuma > 0) {
    float ortalama = toplam / gecerliOkuma;
    
    // Buffer'a ekle
    mesafeBuffer[bufferIndex] = ortalama;
    bufferIndex = (bufferIndex + 1) % MESAFE_BUFFER_SIZE;
    
    // Buffer'daki değerlerin ortalamasını al
    float bufferToplam = 0;
    int bufferGecerli = 0;
    
    for (int i = 0; i < MESAFE_BUFFER_SIZE; i++) {
      if (mesafeBuffer[i] > 2.0 && mesafeBuffer[i] < ayar.maxMesafe) {
        bufferToplam += mesafeBuffer[i];
        bufferGecerli++;
      }
    }
    
    if (bufferGecerli > 0) {
      float sonuc = bufferToplam / bufferGecerli;
      
      // İstatistikleri güncelle
      istatistik.mesafeOkumaSayisi++;
      istatistik.toplamMesafe += sonuc;
      istatistik.ortalamaMesafe = istatistik.toplamMesafe / istatistik.mesafeOkumaSayisi;
      
      if (sonuc > istatistik.maxMesafe) istatistik.maxMesafe = sonuc;
      if (sonuc < istatistik.minMesafe) istatistik.minMesafe = sonuc;
      
      return sonuc;
    }
  }
  
  return sonGecerliMesafe;
}

// =========== GAZ SENSÖRÜ OKUMA ===========
void gazOku() {
  int gazDeger = analogRead(GAS_PIN);
  gaz = map(gazDeger, 0, 4095, 0, 2000);
}

// =========== PIR OKUMA ===========
void pirOku() {
  unsigned long simdi = millis();
  
  if (simdi - sonPirOkuma >= PIR_OKUMA_ARALIGI) {
    int pirDeger = digitalRead(PIR_PIN);
    
    if (pirDeger == HIGH) {
      if (!hareket) {
        hareket = true;
        hareketTespitEdildi = true;
        sonHareketZamani = simdi;
        hareketSayisi++;
        istatistik.hareketSayisi++;
        
        if (ayar.ses && !uyariVar) {
          toneESP(BUZZER_PIN, 1200, 50);
        }
      }
    } else {
      hareket = false;
    }
    
    if (hareketTespitEdildi && (simdi - sonHareketZamani > 2000)) {
      hareketTespitEdildi = false;
    }
    
    sonPirOkuma = simdi;
  }
}

// =========== SENSÖR OKUMA ===========
void sensorOku() {
  unsigned long simdi = millis();
  
  if (simdi - sonMesafeOkuma >= MESAFE_OKUMA_ARALIGI) {
    mesafe = hcSr04OkuFiltreli();
    sonMesafeOkuma = simdi;
  }
  
  gazOku();
  pirOku();
  
  // Sistem bilgilerini güncelle
  sistem.serbestBellek = ESP.getFreeHeap();
  sistem.isikSeviyesi = analogRead(JOY_X);
  sistem.pilSeviyesi = map(analogRead(JOY_Y), 0, 4095, 0, 100);
  
  sonOkuma = simdi;
}

// =========== LED VE BUZZER HIZ GÜNCELLEME ===========
void updateBlinkSpeed() {
  if (ledMod == LED_MOD_MESAFE) {
    if (mesafe >= 200) {
      ledBlinkSpeed = 2000;
      bipInterval = 3000;
    } else if (mesafe >= 150) {
      ledBlinkSpeed = 1500;
      bipInterval = 2000;
    } else if (mesafe >= 100) {
      ledBlinkSpeed = 1200;
      bipInterval = 1500;
    } else if (mesafe >= 70) {
      ledBlinkSpeed = 800;
      bipInterval = 1000;
    } else if (mesafe >= 50) {
      ledBlinkSpeed = 500;
      bipInterval = 700;
    } else if (mesafe >= 30) {
      ledBlinkSpeed = 300;
      bipInterval = 500;
    } else if (mesafe >= 20) {
      ledBlinkSpeed = 200;
      bipInterval = 300;
    } else if (mesafe >= 10) {
      ledBlinkSpeed = 100;
      bipInterval = 200;
    } else {
      ledBlinkSpeed = 50;
      bipInterval = 100;
    }
  }
}

// =========== UYARI KONTROL ===========
void uyariKontrol() {
  bool oncekiUyari = uyariVar;
  
  uyariVar = false;
  uyariSeviye = 0;
  
  if (mesafe < ayar.mesafeTehlike) {
    uyariVar = true;
    uyariSeviye = 2;
    robotIfade = 2;
    sonUyariZamani = millis();
  } else if (mesafe < ayar.mesafeDikkat) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
    sonUyariZamani = millis();
  } else if (gaz > ayar.gazEsik) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
    sonUyariZamani = millis();
  } else {
    robotIfade = 0;
  }
  
  if (uykuModu) {
    robotIfade = 4;
  }
  
  if ((oncekiUyari != uyariVar)) {
    if (!uyariVar && ayar.ses) {
      noToneESP();
      beepCalisiyor = false;
    }
  }
  
  digitalWrite(SYS_LED, uyariVar);
}

// =========== SERVO KONTROLÜ ===========
void servoKontrol() {
  unsigned long simdi = millis();
  
  if (!ayar.servoAktif || !taramaAktif) {
    return;
  }
  
  if (simdi - sonServoHareket < taramaHizi) {
    return;
  }
  
  // Servo sınır kontrolü
  if (servoAci < ayar.servoMinAci) {
    servoAci = ayar.servoMinAci;
  } else if (servoAci > ayar.servoMaxAci) {
    servoAci = ayar.servoMaxAci;
  }
  
  // Mevcut pozisyona git
  if (servoAci != hedefAci) {
    if (servoAci < hedefAci) {
      servoAci++;
    } else {
      servoAci--;
    }
    
    taramaServo.write(servoAci);
    istatistik.servoHareketSayisi++;
  } else {
    // Yeni hedef belirle
    if (!taramaYonu) {
      hedefAci += 10;
      if (hedefAci > ayar.servoMaxAci) {
        hedefAci = ayar.servoMaxAci;
        taramaYonu = true;
        
        if (ayar.ses) {
          toneESP(BUZZER_PIN, 800, 50);
        }
      }
    } else {
      hedefAci -= 10;
      if (hedefAci < ayar.servoMinAci) {
        hedefAci = ayar.servoMinAci;
        taramaYonu = false;
        
        if (ayar.ses) {
          toneESP(BUZZER_PIN, 1200, 50);
        }
      }
    }
    
    hedefAci = constrain(hedefAci, ayar.servoMinAci, ayar.servoMaxAci);
  }
  
  // TARAMA HARİTASI GÜNCELLEMESİ DEĞİŞTİRİLDİ (16 element için)
  int taramaIndex = map(servoAci, ayar.servoMinAci, ayar.servoMaxAci, 0, 15);
  taramaIndex = constrain(taramaIndex, 0, 15);
  
  if (mesafe >= 2.0 && mesafe <= ayar.maxMesafe) {
    if (taramaHaritasi[taramaIndex] == 0) {
      taramaHaritasi[taramaIndex] = mesafe;
    } else {
      taramaHaritasi[taramaIndex] = (taramaHaritasi[taramaIndex] * 0.7) + (mesafe * 0.3);
    }
  }
  
  sonServoHareket = simdi;
}

// =========== JOYSTICK İLE SERVO KONTROL ===========
void joyServoKontrol() {
  unsigned long simdi = millis();
  
  if (simdi - sonJoyServoHareket < joyServoSpeed) {
    return;
  }
  
  int joyX = analogRead(JOY_X);
  
  if (joyX < 1000) {
    servoAci -= 5;
    if (servoAci < ayar.servoMinAci) {
      servoAci = ayar.servoMinAci;
      if (ayar.ses) {
        toneESP(BUZZER_PIN, 800, 50);
      }
    }
    taramaServo.write(servoAci);
    istatistik.servoHareketSayisi++;
    sonJoyServoHareket = simdi;
  } else if (joyX > 3000) {
    servoAci += 5;
    if (servoAci > ayar.servoMaxAci) {
      servoAci = ayar.servoMaxAci;
      if (ayar.ses) {
        toneESP(BUZZER_PIN, 1200, 50);
      }
    }
    taramaServo.write(servoAci);
    istatistik.servoHareketSayisi++;
    sonJoyServoHareket = simdi;
  }
  
  if (digitalRead(JOY_BTN) == LOW) {
    servoAci = SERVO_HOME_POS;
    taramaServo.write(servoAci);
    delay(100);
  }
}

// =========== LED KONTROLÜ ===========
void ledKontrol(unsigned long simdi) {
  if (!ayar.ledAktif) {
    analogWriteESP(LED_PIN, 0);
    return;
  }
  
  switch (ledMod) {
    case LED_MOD_SABIT:
      analogWriteESP(LED_PIN, ledParlaklik);
      break;
      
    case LED_MOD_BLINK:
      if (simdi - sonLedDegisim > ayar.ledHiz) {
        ledDurum = !ledDurum;
        analogWriteESP(LED_PIN, ledDurum ? ledParlaklik : 0);
        sonLedDegisim = simdi;
      }
      break;
      
    case LED_MOD_MESAFE:
      if (simdi - sonLedDegisim > ledBlinkSpeed) {
        ledDurum = !ledDurum;
        
        if (ledDurum) {
          int parlaklik;
          if (mesafe >= 100) parlaklik = 50;
          else if (mesafe >= 50) parlaklik = map(mesafe, 50, 100, 100, 50);
          else if (mesafe >= 20) parlaklik = map(mesafe, 20, 50, 180, 100);
          else parlaklik = 255;
          
          analogWriteESP(LED_PIN, parlaklik);
        } else {
          analogWriteESP(LED_PIN, 0);
        }
        
        sonLedDegisim = simdi;
      }
      break;
      
    case LED_MOD_HAREKET:
      if (hareketTespitEdildi) {
        if (simdi - sonLedDegisim > 200) {
          ledDurum = !ledDurum;
          analogWriteESP(LED_PIN, ledDurum ? ledParlaklik : 0);
          sonLedDegisim = simdi;
        }
      } else {
        analogWriteESP(LED_PIN, 0);
      }
      break;
      
    case LED_MOD_RAINBOW:
      if (simdi - sonLedDegisim > 100) {
        static int rainbowCounter = 0;
        rainbowCounter = (rainbowCounter + 1) % 256;
        int r = sin(rainbowCounter * 0.0245) * 127 + 128;
        int g = sin(rainbowCounter * 0.0245 + 2.094) * 127 + 128;
        int b = sin(rainbowCounter * 0.0245 + 4.188) * 127 + 128;
        analogWriteESP(LED_PIN, (r + g + b) / 3);
        sonLedDegisim = simdi;
      }
      break;
  }
}

// =========== BUZZER KONTROLÜ (DÜZELTİLDİ) ===========
void buzzerKontrol(unsigned long simdi) {
  if (!ayar.ses || !uyariVar || beepCalisiyor) return;
  
  // 3 saniye önceki uyarıları görmezden gel
  if (simdi - sonUyariZamani > 3000) {
    noToneESP();
    return;
  }
  
  // Sadece mesafe tehlike/dikkat eşiklerinde öt
  if (mesafe >= ayar.mesafeDikkat && gaz < ayar.gazEsik) {
    noToneESP();
    return;
  }
  
  if (simdi - sonBip > bipInterval) {
    beepCalisiyor = true;
    
    int bipSuresi;
    int bipFrekans;
    
    if (mesafe < ayar.mesafeTehlike) {
      bipSuresi = 80;
      bipFrekans = 1500;
      bipInterval = 300; // Daha hızlı
    } else if (mesafe < ayar.mesafeDikkat) {
      bipSuresi = 100;
      bipFrekans = 1200;
      bipInterval = 600; // Orta hız
    } else if (gaz > ayar.gazEsik) {
      bipSuresi = 150;
      bipFrekans = 1000;
      bipInterval = 1000; // Yavaş
    } else {
      beepCalisiyor = false;
      return;
    }
    
    toneESP(BUZZER_PIN, bipFrekans, bipSuresi);
    sonBip = simdi;
    
    unsigned long bipBitis = simdi + bipSuresi;
    while (millis() < bipBitis) {
      // Bekle
    }
    beepCalisiyor = false;
  }
}

// =========== BUTON KONTROL ===========
void butonOku() {
  joyBtn = (digitalRead(JOY_BTN) == LOW);
  geriBtn = (digitalRead(GERI_BUTON) == LOW);
}

void butonIsle() {
  static unsigned long sonTiklama = 0;
  unsigned long simdi = millis();
  
  if (simdi - sonTiklama < 300) return;
  
  if (geriBtn && !lastGeriBtn) {
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
  
  if (joyBtn && !lastJoyBtn) {
    sonEtkilesim = simdi;
    uykuModu = false;
    
    switch (aktifMenu) {
      case MENU_ANA:
        switch (menuSecim) {
          case 0: aktifMenu = MENU_MESAFE; break;
          case 1: aktifMenu = MENU_GAZ; break;
          case 2: aktifMenu = MENU_HAREKET; break;
          case 3: aktifMenu = MENU_TARAMA; break;
          case 4: aktifMenu = MENU_SERVO_MANUEL; manuelServoMode = true; break;
          case 5: aktifMenu = MENU_AYAR_MESAFE; break;
          case 6: aktifMenu = MENU_AYAR_GAZ; break;
          case 7: aktifMenu = MENU_AYAR_HAREKET; break;
          case 8: aktifMenu = MENU_AYAR_SES; break;
          case 9: aktifMenu = MENU_AYAR_SERVO; break;
          case 10: aktifMenu = MENU_AYAR_LED; break;
          case 11: aktifMenu = MENU_SISTEM; break;
          case 12: aktifMenu = MENU_GUVENLIK; break;
          case 13: aktifMenu = MENU_KALIBRASYON; break;
          case 14: aktifMenu = MENU_ISTATISTIK; break;
          case 15: aktifMenu = MENU_TEST; break;
          case 16: aktifMenu = MENU_HABERLESME; break;
          case 17: aktifMenu = MENU_GELISMIS; break;
          case 18: aktifMenu = MENU_DIAGNOSTIC; break;
          case 19: aktifMenu = MENU_FIRMWARE; break;
        }
        break;
        
      case MENU_TARAMA:
        taramaAktif = !taramaAktif;
        bipSesi(1);
        break;
        
      case MENU_SERVO_MANUEL:
        servoAci = SERVO_HOME_POS;
        taramaServo.write(servoAci);
        bipSesi(1);
        break;
        
      case MENU_GUVENLIK:
        guvenlikAktif = !guvenlikAktif;
        if (guvenlikAktif) {
          bipSesi(3);
        } else {
          alarmCalisiyor = false;
          noToneESP();
        }
        break;
        
      case MENU_KALIBRASYON:
        kalibrasyonYap();
        break;
        
      case MENU_TEST:
        testModu();
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
        if (!ayar.ses) {
          noToneESP();
          beepCalisiyor = false;
        }
        bipSesi(1);
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
  
  if (simdi - sonHareket < 200) return;
  
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  bool hareketVar = false;
  
  if (joyY < 1000) {
    hareketVar = true;
    if (aktifMenu == MENU_ANA) {
      menuSecim--;
      if (menuSecim < 0) menuSecim = TOPLAM_MENU_OGE - 1;
      if (menuSecim < menuBaslangic) {
        menuBaslangic = menuSecim;
      } else if (menuSecim >= menuBaslangic + MAX_GORUNEN_OGE) {
        menuBaslangic = menuSecim - MAX_GORUNEN_OGE + 1;
      }
    } else if (ayarModu) {
      ayarSecim--;
    }
  } else if (joyY > 3000) {
    hareketVar = true;
    if (aktifMenu == MENU_ANA) {
      menuSecim++;
      if (menuSecim >= TOPLAM_MENU_OGE) menuSecim = 0;
      if (menuSecim < menuBaslangic) {
        menuBaslangic = menuSecim;
      } else if (menuSecim >= menuBaslangic + MAX_GORUNEN_OGE) {
        menuBaslangic = menuSecim - MAX_GORUNEN_OGE + 1;
      }
    } else if (ayarModu) {
      ayarSecim++;
    }
  }
  
  if (ayarModu && (joyX > 3000 || joyX < 1000)) {
    hareketVar = true;
    ayarDegistir(joyX > 3000 ? 1 : -1);
  }
  
  if (hareketVar) {
    sonEtkilesim = simdi;
    uykuModu = false;
    bipSesi(1);
    sonHareket = simdi;
  }
}

void ayarDegistir(int yon) {
  switch (aktifMenu) {
    case MENU_AYAR_MESAFE:
      if (ayarSecim == 0) {
        ayar.mesafeDikkat += yon * 5;
        ayar.mesafeDikkat = constrain(ayar.mesafeDikkat, 20, 200);
      } else if (ayarSecim == 1) {
        ayar.mesafeTehlike += yon * 5;
        ayar.mesafeTehlike = constrain(ayar.mesafeTehlike, 5, 100);
      } else {
        ayar.maxMesafe += yon * 50;
        ayar.maxMesafe = constrain(ayar.maxMesafe, 100, 500);
      }
      break;
      
    case MENU_AYAR_GAZ:
      ayar.gazEsik += yon * 50;
      ayar.gazEsik = constrain(ayar.gazEsik, 300, 2000);
      break;
      
    case MENU_AYAR_SERVO:
      if (ayarSecim == 0) {
        ayar.servoSpeed += yon * 1;
        ayar.servoSpeed = constrain(ayar.servoSpeed, 5, 50);
        taramaHizi = ayar.servoSpeed;
      } else if (ayarSecim == 1) {
        ayar.servoMinAci += yon * 5;
        // DEĞİŞTİRİLDİ: Min 0, Max servoMaxAci-10
        ayar.servoMinAci = constrain(ayar.servoMinAci, 0, ayar.servoMaxAci - 10);
      } else if (ayarSecim == 2) {
        ayar.servoMaxAci += yon * 5;
        // DEĞİŞTİRİLDİ: Min servoMinAci+10, Max 150
        ayar.servoMaxAci = constrain(ayar.servoMaxAci, ayar.servoMinAci + 10, 150);
      }
      break;
      
    case MENU_AYAR_LED:
      if (ayarSecim == 0) {
        ledMod += yon;
        if (ledMod > 4) ledMod = 0;
        if (ledMod < 0) ledMod = 4;
        ayar.ledMode = ledMod;
      } else if (ayarSecim == 1) {
        ayar.ledHiz += yon * 50;
        ayar.ledHiz = constrain(ayar.ledHiz, 100, 1000);
      } else if (ayarSecim == 2) {
        ayar.ledBright += yon * 10;
        ayar.ledBright = constrain(ayar.ledBright, 0, 255);
        ledParlaklik = ayar.ledBright;
      }
      break;
  }
  bipSesi(1);
}

// =========== KALİBRASYON ===========
void kalibrasyonYap() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.print("KALIBRASYON YAPILIYOR");
  display.display();
  
  // Mesafe sensörü kalibrasyonu
  float toplam = 0;
  for (int i = 0; i < 10; i++) {
    toplam += hcSr04Oku();
    delay(50);
  }
  float ortalama = toplam / 10;
  
  // Gaz sensörü kalibrasyonu
  int gazTemiz = analogRead(GAS_PIN);
  
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("Kalibrasyon Tamam!");
  display.setCursor(10, 30);
  display.print("Mesafe: ");
  display.print(ortalama);
  display.print(" cm");
  display.setCursor(10, 40);
  display.print("Gaz: ");
  display.print(gazTemiz);
  display.display();
  
  bipSesi(3);
  delay(2000);
}

// =========== TEST MODU ===========
void testModu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.print("TEST MODU");
  display.setCursor(10, 25);
  display.print("Servo testi...");
  display.display();
  
  // Servo testi
  for (int i = ayar.servoMinAci; i <= ayar.servoMaxAci; i += 10) {
    taramaServo.write(i);
    delay(100);
  }
  for (int i = ayar.servoMaxAci; i >= ayar.servoMinAci; i -= 10) {
    taramaServo.write(i);
    delay(100);
  }
  taramaServo.write(SERVO_HOME_POS);
  
  // LED testi
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("LED testi...");
  display.display();
  
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  // Buzzer testi
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("Buzzer testi...");
  display.display();
  
  for (int i = 200; i < 2000; i += 100) {
    toneESP(BUZZER_PIN, i, 50);
    delay(50);
  }
  noToneESP();
  
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("Test tamamlandi!");
  display.display();
  delay(1000);
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
  switch (aktifMenu) {
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
    case MENU_KALIBRASYON: display.print("KALIBRASYON"); break;
    case MENU_ISTATISTIK: display.print("ISTATISTIK"); break;
    case MENU_TEST: display.print("TEST"); break;
    case MENU_HABERLESME: display.print("HABERLESME"); break;
    case MENU_GELISMIS: display.print("GELISMIS"); break;
    case MENU_DIAGNOSTIC: display.print("DIAGNOSTIC"); break;
    case MENU_FIRMWARE: display.print("FIRMWARE"); break;
  }
}

void icerikCiz() {
  switch (aktifMenu) {
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
    case MENU_KALIBRASYON: kalibrasyonEkranCiz(); break;
    case MENU_ISTATISTIK: istatistikEkranCiz(); break;
    case MENU_TEST: testEkranCiz(); break;
    case MENU_HABERLESME: haberlesmeEkranCiz(); break;
    case MENU_GELISMIS: gelismisEkranCiz(); break;
    case MENU_DIAGNOSTIC: diagnosticEkranCiz(); break;
    case MENU_FIRMWARE: firmwareEkranCiz(); break;
  }
}

void anaMenuCiz() {
  const char* menuItems[] = {
    "Mesafe Goster",
    "Gaz Goster",
    "Hareket Goster",
    "Tarama Ekrani",
    "Servo Manuel Kontrol",
    "Mesafe Ayari",
    "Gaz Ayari",
    "Hareket Ayari",
    "Ses Ayari",
    "Servo Ayari",
    "LED Ayari",
    "Sistem Bilgisi",
    "Guvenlik Sistemi",
    "Kalibrasyon",
    "Istatistikler",
    "Test Modu",
    "Haberlesme",
    "Gelismis Ayarlar",
    "Diagnostic",
    "Firmware"
  };
  
  int baslangic = menuBaslangic;
  int bitis = min(baslangic + MAX_GORUNEN_OGE, TOPLAM_MENU_OGE);
  
  for (int i = baslangic; i < bitis; i++) {
    int listIndex = i - baslangic;
    int y = 15 + listIndex * 9;
    
    if (i == menuSecim) {
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
  
  if (TOPLAM_MENU_OGE > MAX_GORUNEN_OGE) {
    int scrollBarHeight = 40;
    int scrollBarY = 15;
    int scrollPos = map(menuBaslangic, 0, TOPLAM_MENU_OGE - MAX_GORUNEN_OGE, scrollBarY, scrollBarY + scrollBarHeight - 5);
    display.drawRect(125, scrollBarY, 2, scrollBarHeight, SSD1306_WHITE);
    display.fillRect(125, scrollPos, 2, 5, SSD1306_WHITE);
  }
}

void mesafeEkranCiz() {
  display.setTextSize(3);
  display.setCursor(15, 15);
  
  if (mesafe >= 100) {
    display.print(mesafe, 0);
  } else if (mesafe >= 10) {
    display.print(mesafe, 0);
  } else {
    display.print(mesafe, 1);
  }
  
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("cm");
  
  display.setCursor(5, 55);
  display.print("Max: ");
  display.print(ayar.maxMesafe);
  display.print("cm  ");
  display.print(mesafeSensorCalisiyor ? "OK" : "HATA");
}

void gazEkranCiz() {
  display.setTextSize(3);
  display.setCursor(15, 15);
  display.print(gaz, 0);
  
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("ppm");
  
  display.setCursor(5, 55);
  if (gaz < 500) display.print("HAVA KALITESI: IYI");
  else if (gaz < 1000) display.print("HAVA KALITESI: ORTA");
  else display.print("HAVA KALITESI: KOTU");
}

void hareketEkranCiz() {
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print(hareket ? "HAREKET VAR" : "HAREKET YOK");
  
  display.setTextSize(1);
  display.setCursor(5, 45);
  display.print("Toplam: ");
  display.print(hareketSayisi);
  
  display.setCursor(5, 55);
  display.print("Son: ");
  if (sonHareketZamani > 0) {
    unsigned long gecenSure = (millis() - sonHareketZamani) / 1000;
    display.print(gecenSure);
    display.print(" sn once");
  } else {
    display.print("--");
  }
}

// =========== TARAMA EKRANI TANK RADAR GÖRÜNÜMÜ ===========
void taramaEkranCiz() {
  int centerX = 64;
  int centerY = 30;
  int radius = 24;
  
  // Radar alanını temizle (y=12'den y=48'e kadar)
  display.fillRect(0, 12, 128, 36, SSD1306_BLACK);
  
  // Mesafe halkalarını çiz (50cm, 100cm, 150cm, 200cm)
  for (int d = 50; d <= 200; d += 50) {
    int ringRadius = map(d, 0, 200, 0, radius);
    // 0-180 derece arası yay çiz
    for (int angle = 0; angle <= 180; angle += 5) {
      float rad = radians(angle);
      int x = centerX + sin(rad) * ringRadius;
      int y = centerY - cos(rad) * ringRadius;
      display.drawPixel(x, y, SSD1306_WHITE);
    }
  }
  
  // Mevcut tarama çizgisini çiz
  float currentRad = radians(servoAci);
  int lineX = centerX + sin(currentRad) * radius;
  int lineY = centerY - cos(currentRad) * radius;
  display.drawLine(centerX, centerY, lineX, lineY, SSD1306_WHITE);
  
  // Kayıtlı noktaları çiz (blips)
  for (int i = 0; i < 16; i++) {
    float mesafeDegeri = taramaHaritasi[i];
    if (mesafeDegeri > 2.0 && mesafeDegeri < ayar.maxMesafe) {
      // i 0, 10, 20, ..., 150 derecelere karşılık gelir
      int angleDeg = i * 10;
      // Açıyı 0-150'den 0-180'e haritala (görüntü için)
      int displayAngle = map(angleDeg, 0, 150, 0, 180);
      float rad = radians(displayAngle);
      int blipRadius = map(mesafeDegeri, 0, 200, 0, radius);
      int blipX = centerX + sin(rad) * blipRadius;
      int blipY = centerY - cos(rad) * blipRadius;
      
      // Nokta çiz
      if (mesafeDegeri < ayar.mesafeTehlike) {
        display.fillCircle(blipX, blipY, 2, SSD1306_WHITE);
      } else if (mesafeDegeri < ayar.mesafeDikkat) {
        display.drawCircle(blipX, blipY, 2, SSD1306_WHITE);
      } else {
        display.drawPixel(blipX, blipY, SSD1306_WHITE);
      }
    }
  }
  
  // Alt bilgi çubuğu
  display.fillRect(0, 48, 128, 16, SSD1306_BLACK);
  display.drawFastHLine(0, 48, 128, SSD1306_WHITE);
  
  display.setCursor(2, 50);
  display.print("A:");
  display.print(servoAci);
  display.print(" M:");
  display.print(mesafe, 0);
  display.print("cm");
  
  display.setCursor(70, 50);
  display.print("Tarama:");
  display.print(taramaAktif ? "ON" : "OFF");
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
  display.print("JOY:Sol/Sag  BTN:Orta");
}

void ayarMesafeCiz() {
  if (ayarModu) {
    display.setCursor(5, 15);
    if (ayarSecim == 0) {
      display.print("Dikkat:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.mesafeDikkat);
      display.print("cm");
    } else if (ayarSecim == 1) {
      display.print("Tehlike:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.mesafeTehlike);
      display.print("cm");
    } else {
      display.print("Max Mesafe:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.maxMesafe);
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
    display.setCursor(5, 35);
    display.print("Max: ");
    display.print(ayar.maxMesafe);
    display.print("cm");
    display.setCursor(5, 45);
    display.print("Yukari/Asagi: Sec");
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarGazCiz() {
  if (ayarModu) {
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
  if (ayarModu) {
    display.setCursor(5, 15);
    display.print("Hareket Esik:");
    display.setTextSize(2);
    display.setCursor(30, 30);
    display.print(ayar.hareketEsikSure/1000);
    display.print(" sn");
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Esik Sure: ");
    display.print(ayar.hareketEsikSure/1000);
    display.print(" sn");
    display.setCursor(5, 25);
    display.print("Hareket: ");
    display.print(hareket ? "VAR" : "YOK");
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
  if (ayarModu) {
    display.setCursor(5, 15);
    if (ayarSecim == 0) {
      display.print("Tarama Hizi:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoSpeed);
      display.print("ms");
    } else if (ayarSecim == 1) {
      display.print("Min Aci:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoMinAci);
      display.print("°");
    } else if (ayarSecim == 2) {
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
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarLedCiz() {
  if (ayarModu) {
    display.setCursor(5, 15);
    if (ayarSecim == 0) {
      display.print("LED Modu:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      switch(ledMod) {
        case 0: display.print("SABIT"); break;
        case 1: display.print("BLINK"); break;
        case 2: display.print("MESAFE"); break;
        case 3: display.print("HAREKET"); break;
        case 4: display.print("RAINBOW"); break;
      }
    } else if (ayarSecim == 1) {
      display.print("LED Hizi:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.ledHiz);
      display.print("ms");
    } else if (ayarSecim == 2) {
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
    switch(ledMod) {
      case 0: display.print("SABIT"); break;
      case 1: display.print("BLINK"); break;
      case 2: display.print("MESAFE"); break;
      case 3: display.print("HAREKET"); break;
      case 4: display.print("RAINBOW"); break;
    }
    display.setCursor(5, 25);
    display.print("Hiz: ");
    display.print(ayar.ledHiz);
    display.print("ms");
    display.setCursor(5, 35);
    display.print("Parlaklik: ");
    display.print(ayar.ledBright);
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void sistemEkranCiz() {
  display.setCursor(5, 10);
  display.print("TOPROAK PRO MAX V7.2");
  display.setCursor(5, 20);
  display.print("Calisma: ");
  display.print((millis() - istatistik.baslangicZamani) / 1000);
  display.print(" sn");
  display.setCursor(5, 30);
  display.print("Pil: ");
  display.print(sistem.pilSeviyesi, 0);
  display.print("%");
  display.setCursor(5, 40);
  display.print("Isik: ");
  display.print(sistem.isikSeviyesi);
  display.setCursor(5, 50);
  display.print("Bellek: ");
  display.print(sistem.serbestBellek);
  display.print(" B");
}

void guvenlikEkranCiz() {
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print(guvenlikAktif ? "AKTIF" : "PASIF");
  
  display.setTextSize(1);
  display.setCursor(5, 45);
  if (guvenlikAktif) {
    display.print("Durum: ");
    if (alarmCalisiyor) {
      display.print("ALARM!");
    } else {
      display.print("BEKLIYOR");
    }
  } else {
    display.print("Guvenlik sistemi");
    display.setCursor(5, 55);
    display.print("pasif durumda");
  }
}

void kalibrasyonEkranCiz() {
  display.setCursor(20, 20);
  display.print("KALIBRASYON");
  display.setCursor(10, 35);
  display.print("Mesafe sensoru ve");
  display.setCursor(10, 45);
  display.print("gaz sensoru");
  display.setCursor(30, 55);
  display.print("kalibre edilecek");
}

void istatistikEkranCiz() {
  display.setCursor(5, 10);
  display.print("Mesafe Okuma: ");
  display.print(istatistik.mesafeOkumaSayisi);
  display.setCursor(5, 20);
  display.print("Max Mesafe: ");
  display.print(istatistik.maxMesafe, 0);
  display.print("cm");
  display.setCursor(5, 30);
  display.print("Min Mesafe: ");
  display.print(istatistik.minMesafe, 0);
  display.print("cm");
  display.setCursor(5, 40);
  display.print("Ortalama: ");
  display.print(istatistik.ortalamaMesafe, 0);
  display.print("cm");
  display.setCursor(5, 50);
  display.print("Hareket: ");
  display.print(istatistik.hareketSayisi);
  display.setCursor(5, 60);
  display.print("Servo Hareket: ");
  display.print(istatistik.servoHareketSayisi);
}

void testEkranCiz() {
  display.setCursor(30, 20);
  display.print("TEST MODU");
  display.setCursor(10, 35);
  display.print("Servo, LED, Buzzer");
  display.setCursor(20, 45);
  display.print("test edilecek");
  display.setCursor(40, 55);
  display.print("Buton: Baslat");
}

void haberlesmeEkranCiz() {
  display.setCursor(20, 20);
  display.print("HABERLESME");
  display.setCursor(10, 35);
  display.print("Seri Port: ");
  display.print(Serial.baudRate());
  display.setCursor(10, 45);
  display.print("WiFi: Kapali");
  display.setCursor(10, 55);
  display.print("Bluetooth: Kapali");
}

void gelismisEkranCiz() {
  display.setCursor(20, 20);
  display.print("GELISMIS");
  display.setCursor(10, 35);
  display.print("Otomatik Kalibrasyon");
  display.setCursor(10, 45);
  display.print("Veri Kaydi: Kapali");
  display.setCursor(10, 55);
  display.print("Gunluk: Kapali");
}

void diagnosticEkranCiz() {
  display.setCursor(20, 20);
  display.print("DIAGNOSTIC");
  display.setCursor(10, 35);
  display.print("Sensor Testi");
  display.setCursor(10, 45);
  display.print("Bellek Testi");
  display.setCursor(10, 55);
  display.print("Performans Testi");
}

void firmwareEkranCiz() {
  display.setCursor(20, 20);
  display.print("FIRMWARE");
  display.setCursor(10, 35);
  display.print("Versiyon: V7.2");
  display.setCursor(10, 45);
  display.print("Derleme: 2024");
  display.setCursor(10, 55);
  display.print("Guncelleme: Yok");
}

void robotCiz(int x, int y) {
  display.drawCircle(x, y, 6, SSD1306_WHITE);
  
  if (gozAcik) {
    if (robotIfade == 0) {
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawPixel(x, y+2, SSD1306_WHITE);
    } else if (robotIfade == 2) {
      display.fillCircle(x-3, y-2, 2, SSD1306_WHITE);
      display.fillCircle(x+3, y-2, 2, SSD1306_WHITE);
      display.drawLine(x-3, y+2, x+3, y+2, SSD1306_WHITE);
    }
  } else {
    display.drawLine(x-3, y-1, x-1, y-1, SSD1306_WHITE);
    display.drawLine(x+1, y-1, x+3, y-1, SSD1306_WHITE);
  }
  
  if (uyariVar) {
    display.drawTriangle(x+8, y-8, x+8, y-3, x+5, y-5, SSD1306_WHITE);
  }
}

void altBilgiCiz() {
  display.drawFastHLine(0, 63, 128, SSD1306_WHITE);
  
  if (uyariVar) {
    display.setCursor(5, 56);
    display.print("UYARI! Mesafe:");
    display.print(mesafe, 0);
    display.print("cm");
  } else {
    display.setCursor(5, 56);
    display.print("D18:Ana Menu");
    display.setCursor(85, 56);
    display.print("JOY:Sec");
  }
}

// =========== SES FONKSİYONLARI ===========
void bipSesi(int tip) {
  if (!ayar.ses) return;
  
  switch (tip) {
    case 1:
      toneESP(BUZZER_PIN, 800, 50);
      break;
    case 2:
      toneESP(BUZZER_PIN, 1000, 100);
      break;
    case 3:
      toneESP(BUZZER_PIN, 400, 200);
      break;
  }
}

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  Serial.println("TOPROAK PRO MAX V7.2 baslatiliyor...");
  
  // Pin modları
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(GERI_BUTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SYS_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(SYS_LED, LOW);
  
  // ESP32 PWM Ayarları
  ledcSetup(BUZZER_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);
  
  ledcSetup(LED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
  ledcWrite(LED_CHANNEL, 0);
  
  // Mesafe buffer'ı başlat
  for (int i = 0; i < MESAFE_BUFFER_SIZE; i++) {
    mesafeBuffer[i] = 100.0;
  }
  
  // Tarama haritasını sıfırla
  for (int i = 0; i < 16; i++) {
    taramaHaritasi[i] = 0;
  }
  
  // SERVO BAŞLATMA DÜZELTİLDİ
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  taramaServo.setPeriodHertz(50);  // Standart 50hz servo
  taramaServo.attach(SERVO_PIN, 500, 2500);  // Servo pin ve min/max pulse width
  
  // Servo testi - DEĞİŞTİRİLDİ: 0-150 derece arasında test
  servoAci = SERVO_HOME_POS;
  hedefAci = SERVO_HOME_POS;
  taramaServo.write(servoAci);
  delay(500);
  
  // OLED başlatma
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED hatasi!");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
  }
  
  // Başlangıç ekranı
  baslangicEkrani();
  
  // İstatistik başlangıcı
  istatistik.baslangicZamani = millis();
  
  Serial.println("TOPROAK PRO MAX V7.2 - Sistem hazir!");
  Serial.print("Servo Min: "); Serial.print(ayar.servoMinAci);
  Serial.print(" Max: "); Serial.println(ayar.servoMaxAci);
}

void baslangicEkrani() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPROAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("PRO MAX V7.2");
  display.display();
  
  delay(2000);
  
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("5m Mesafe Sensoru");
  display.setCursor(20, 35);
  display.print("Aktif Edildi");
  display.setCursor(15, 50);
  display.print("Servo 0-150 Derece");
  display.display();
  
  delay(1000);
}

// =========== ANA DÖNGÜ ===========
void loop() {
  unsigned long simdi = millis();
  
  // Sensör okuma
  if (simdi - sonOkuma > 50) {
    sensorOku();
    uyariKontrol();
    updateBlinkSpeed();
    sonOkuma = simdi;
  }
  
  // Buton okuma ve işleme
  butonOku();
  butonIsle();
  
  // Joystick kontrol
  if (!manuelServoMode) {
    joyKontrol();
  }
  
  // Servo kontrol
  if (ayar.servoAktif) {
    if (manuelServoMode) {
      joyServoKontrol();
    } else if (taramaAktif) {
      servoKontrol();
    }
  }
  
  // LED kontrol
  ledKontrol(simdi);
  
  // Buzzer kontrol
  buzzerKontrol(simdi);
  
  // Ekran güncelleme
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    ekranCiz();
  }
  
  // Göz kırpma
  if (simdi - sonGozKirpma > 3000 + random(0, 2000)) {
    gozAcik = !gozAcik;
    sonGozKirpma = simdi;
  }
  
  // Uyku modu
  if (simdi - sonEtkilesim > 20000 && !uykuModu && !guvenlikAktif) {
    uykuModu = true;
    robotIfade = 4;
  } else if (simdi - sonEtkilesim < 20000 && uykuModu) {
    uykuModu = false;
    robotIfade = 0;
  }
  
  // İstatistik güncelleme
  istatistik.calismaSuresi = millis() - istatistik.baslangicZamani;
  
  delay(10);
}

// =========== SERİAL KOMUT İŞLEME ===========
void serialEvent() {
  while (Serial.available()) {
    char c = Serial.read();
    
    if (c == '\n') {
      String command = Serial.readStringUntil('\n');
      command.trim();
      
      if (command == "status") {
        Serial.println("=== TOPROAK V7.2 STATUS ===");
        Serial.print("Mesafe: ");
        Serial.print(mesafe);
        Serial.println(" cm");
        Serial.print("Gaz: ");
        Serial.print(gaz);
        Serial.println(" ppm");
        Serial.print("Hareket: ");
        Serial.println(hareket ? "VAR" : "YOK");
        Serial.print("Servo Aci: ");
        Serial.print(servoAci);
        Serial.println(" derece");
        Serial.print("LED Mod: ");
        Serial.println(ledMod);
        Serial.print("Tarama: ");
        Serial.println(taramaAktif ? "AKTIF" : "PASIF");
        Serial.print("Guvenlik: ");
        Serial.println(guvenlikAktif ? "AKTIF" : "PASIF");
        Serial.println("========================");
      } else if (command == "servo on") {
        ayar.servoAktif = true;
        Serial.println("Servo ACILDI");
      } else if (command == "servo off") {
        ayar.servoAktif = false;
        Serial.println("Servo KAPATILDI");
      } else if (command.startsWith("servo ")) {
        int angle = command.substring(6).toInt();
        servoAci = constrain(angle, ayar.servoMinAci, ayar.servoMaxAci);
        taramaServo.write(servoAci);
        Serial.print("Servo ");
        Serial.print(servoAci);
        Serial.println(" derece");
      } else if (command == "scan on") {
        taramaAktif = true;
        Serial.println("Tarama ACILDI");
      } else if (command == "scan off") {
        taramaAktif = false;
        Serial.println("Tarama KAPATILDI");
      } else if (command == "led on") {
        ayar.ledAktif = true;
        Serial.println("LED ACILDI");
      } else if (command == "led off") {
        ayar.ledAktif = false;
        Serial.println("LED KAPATILDI");
      } else if (command == "buzzer on") {
        ayar.ses = true;
        Serial.println("Buzzer ACILDI");
      } else if (command == "buzzer off") {
        ayar.ses = false;
        Serial.println("Buzzer KAPATILDI");
      } else if (command == "security on") {
        guvenlikAktif = true;
        Serial.println("Guvenlik ACILDI");
      } else if (command == "security off") {
        guvenlikAktif = false;
        Serial.println("Guvenlik KAPATILDI");
      } else if (command == "calibrate") {
        kalibrasyonYap();
      } else if (command == "test") {
        testModu();
      } else if (command == "help") {
        Serial.println("TOPROAK V7.2 Komutlari:");
        Serial.println("status - Sistem durumu");
        Serial.println("servo on/off - Servo ac/kapa");
        Serial.println("servo [aci] - Servo aci ayarla");
        Serial.println("scan on/off - Tarama ac/kapa");
        Serial.println("led on/off - LED ac/kapa");
        Serial.println("buzzer on/off - Buzzer ac/kapa");
        Serial.println("security on/off - Guvenlik ac/kapa");
        Serial.println("calibrate - Kalibrasyon yap");
        Serial.println("test - Test modu");
        Serial.println("help - Bu mesaji goster");
      }
    }
  }
}
