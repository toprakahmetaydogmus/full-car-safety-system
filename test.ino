/****************************************************
 * TOPRAK - Akıllı Araba Asistanı PRO MAX V5.5
 * TÜM SORUNLAR ÇÖZÜLMÜŞ GELİŞTİRİLMİŞ SÜRÜM
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

// =========== DEĞİŞKENLER ===========
// Sensörler
float mesafe = 0;
float gaz = 0;

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
bool hareket = false;
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
const int TOPLAM_MENU_OGE = 14;

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0;
unsigned long sonUyariZamani = 0;
bool beepCalisiyor = false;

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0;

// =========== SERVO SİSTEMİ - DÜZELTİLMİŞ ===========
// Servo sınırları (güvenli aralık)
const int SERVO_MIN_LIMIT = 30;     // Fiziksel sağ sınır
const int SERVO_MAX_LIMIT = 150;    // Fiziksel sol sınır
const int SERVO_HOME_POS = 90;      // Orta nokta

int servoAci = SERVO_HOME_POS;
int hedefAci = SERVO_HOME_POS;
bool taramaYonu = false; // false = sağa, true = sola
bool taramaAktif = true;
int taramaHizi = 15;
float taramaHaritasi[37] = {0};
unsigned long sonServoHareket = 0;
bool alarmModu = false;

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

// Ayarlar
struct {
  int mesafeDikkat = 50;
  int mesafeTehlike = 20;
  int gazEsik = 800;
  int hareketEsikSure = 3000;
  bool ses = true;
  
  bool servoAktif = true;
  int servoSpeed = 15;
  int servoMinAci = SERVO_MIN_LIMIT;
  int servoMaxAci = SERVO_MAX_LIMIT;
  
  bool ledAktif = true;
  int ledMode = 2;
  int ledHiz = 500;
  int ledBright = 200;
  
  bool guvenlikOtomatik = false;
} ayar;

// Zamanlayıcılar
unsigned long sonOkuma = 0;
unsigned long sonEtkilesim = 0;
bool uykuModu = false;

// =========== GELİŞMİŞ HC-SR04 OKUMA ===========
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
      Serial.println("HC-SR04 HATA: Timeout!");
    }
    return sonGecerliMesafe;
  }
  
  float distance = duration * 0.0343 / 2; // Daha hassas sabit
  
  if (distance < 2.0 || distance > 400.0) {
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
  
  for (int i = 0; i < 5; i++) { // 5 okuma yap
    float okuma = hcSr04Oku();
    
    if (okuma >= 2.0 && okuma <= 400.0) {
      if (i > 0 && abs(okuma - toplam/gecerliOkuma) > 100.0) {
        // 100cm'den fazla ani değişim, geçersiz say
        continue;
      }
      
      toplam += okuma;
      gecerliOkuma++;
    }
    delay(10); // Okumalar arası bekle
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
      if (mesafeBuffer[i] > 2.0 && mesafeBuffer[i] < 400.0) {
        bufferToplam += mesafeBuffer[i];
        bufferGecerli++;
      }
    }
    
    if (bufferGecerli > 0) {
      float sonuc = bufferToplam / bufferGecerli;
      Serial.print("Mesafe: ");
      Serial.print(sonuc);
      Serial.println(" cm");
      return sonuc;
    }
  }
  
  Serial.println("Mesafe okunamadi, son gecerli deger kullaniliyor");
  return sonGecerliMesafe;
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
        
        Serial.println("HAREKET TESPIT EDILDI!");
        
        if (ayar.ses && !uyariVar) {
          tone(BUZZER_PIN, 1200, 50);
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
    updateBlinkSpeed();
  }
  
  int gazDeger = analogRead(GAS_PIN);
  gaz = map(gazDeger, 0, 4095, 0, 2000);
  
  pirOku();
}

// =========== LED VE BUZZER HIZ GÜNCELLEME ===========
void updateBlinkSpeed() {
  if (ledMod == LED_MOD_MESAFE) {
    if (mesafe >= 200) {
      ledBlinkSpeed = 2000;
      bipInterval = 2500;
    } else if (mesafe >= 150) {
      ledBlinkSpeed = 1500;
      bipInterval = 2000;
    } else if (mesafe >= 100) {
      ledBlinkSpeed = 1000;
      bipInterval = 1500;
    } else if (mesafe >= 70) {
      ledBlinkSpeed = 700;
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
      noTone(BUZZER_PIN);
      beepCalisiyor = false;
    }
  }
  
  digitalWrite(SYS_LED, uyariVar);
}

// =========== SERVO KONTROLÜ - GELİŞTİRİLMİŞ ===========
void servoKontrol() {
  unsigned long simdi = millis();
  
  if (!ayar.servoAktif || !taramaAktif) {
    return;
  }
  
  if (simdi - sonServoHareket < taramaHizi) {
    return;
  }
  
  // Servo sınır kontrolü
  if (servoAci < SERVO_MIN_LIMIT) {
    servoAci = SERVO_MIN_LIMIT;
    Serial.println("SERVO SAG SINIRA ULASILDI");
  } else if (servoAci > SERVO_MAX_LIMIT) {
    servoAci = SERVO_MAX_LIMIT;
    Serial.println("SERVO SOL SINIRA ULASILDI");
  }
  
  // Mevcut pozisyona git
  if (servoAci != hedefAci) {
    if (servoAci < hedefAci) {
      servoAci++;
    } else {
      servoAci--;
    }
    
    taramaServo.write(servoAci);
  } else {
    // Yeni hedef belirle
    if (!taramaYonu) { // SAĞA gidiyor (artıyor)
      hedefAci += 10;
      if (hedefAci > ayar.servoMaxAci) {
        hedefAci = ayar.servoMaxAci;
        taramaYonu = true; // SOLA dön (azalt)
        
        if (ayar.ses) {
          tone(BUZZER_PIN, 800, 50);
        }
        Serial.println("TARAMA YONU DEGISTI: SOLA");
      }
    } else { // SOLA gidiyor (azalıyor)
      hedefAci -= 10;
      if (hedefAci < ayar.servoMinAci) {
        hedefAci = ayar.servoMinAci;
        taramaYonu = false; // SAĞA dön (artır)
        
        if (ayar.ses) {
          tone(BUZZER_PIN, 1200, 50);
        }
        Serial.println("TARAMA YONU DEGISTI: SAGA");
      }
    }
    
    // Hedef açıyı güvenli sınırlara getir
    hedefAci = constrain(hedefAci, ayar.servoMinAci, ayar.servoMaxAci);
  }
  
  // GELİŞMİŞ TARAMA HARİTASI
  int taramaIndex = map(servoAci, ayar.servoMinAci, ayar.servoMaxAci, 0, 36);
  taramaIndex = constrain(taramaIndex, 0, 36);
  
  // Mesafe kaydet (önceki değerlerle ortalamasını al)
  if (mesafe >= 2.0 && mesafe <= 400.0) {
    if (taramaHaritasi[taramaIndex] == 0) {
      taramaHaritasi[taramaIndex] = mesafe;
    } else {
      // Eski ve yeni değerin ortalamasını al
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
  
  // Joystick sol - servo sola (azalt)
  if (joyX < 1000) {
    servoAci -= 5;
    if (servoAci < SERVO_MIN_LIMIT) {
      servoAci = SERVO_MIN_LIMIT;
      if (ayar.ses) {
        tone(BUZZER_PIN, 800, 50);
      }
      Serial.println("SERVO SAG SINIRA ULASILDI (JOY)");
    }
    taramaServo.write(servoAci);
    sonJoyServoHareket = simdi;
  } 
  // Joystick sağ - servo sağa (artır)
  else if (joyX > 3000) {
    servoAci += 5;
    if (servoAci > SERVO_MAX_LIMIT) {
      servoAci = SERVO_MAX_LIMIT;
      if (ayar.ses) {
        tone(BUZZER_PIN, 1200, 50);
      }
      Serial.println("SERVO SOL SINIRA ULASILDI (JOY)");
    }
    taramaServo.write(servoAci);
    sonJoyServoHareket = simdi;
  }
  
  // Joystick butonu ile orta noktaya dön
  if (digitalRead(JOY_BTN) == LOW) {
    servoAci = SERVO_HOME_POS;
    taramaServo.write(servoAci);
    delay(100);
  }
}

// =========== LED KONTROLÜ ===========
void ledKontrol(unsigned long simdi) {
  if (!ayar.ledAktif) {
    analogWrite(LED_PIN, 0);
    return;
  }
  
  switch (ledMod) {
    case LED_MOD_SABIT:
      analogWrite(LED_PIN, ledParlaklik);
      break;
      
    case LED_MOD_BLINK:
      if (simdi - sonLedDegisim > ayar.ledHiz) {
        ledDurum = !ledDurum;
        analogWrite(LED_PIN, ledDurum ? ledParlaklik : 0);
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
          
          analogWrite(LED_PIN, parlaklik);
        } else {
          analogWrite(LED_PIN, 0);
        }
        
        sonLedDegisim = simdi;
      }
      break;
      
    case LED_MOD_HAREKET:
      if (hareketTespitEdildi) {
        // Hareket varsa hızlı yanıp sön
        if (simdi - sonLedDegisim > 200) {
          ledDurum = !ledDurum;
          analogWrite(LED_PIN, ledDurum ? ledParlaklik : 0);
          sonLedDegisim = simdi;
        }
      } else {
        analogWrite(LED_PIN, 0);
      }
      break;
  }
}

// =========== BUZZER KONTROLÜ ===========
void buzzerKontrol(unsigned long simdi) {
  if (!ayar.ses || !uyariVar || beepCalisiyor) return;
  
  if (simdi - sonUyariZamani > 3000) {
    noTone(BUZZER_PIN);
    return;
  }
  
  if (simdi - sonBip > bipInterval) {
    beepCalisiyor = true;
    
    int bipSuresi;
    if (mesafe >= 100) bipSuresi = 100;
    else if (mesafe >= 50) bipSuresi = 80;
    else if (mesafe >= 20) bipSuresi = 60;
    else bipSuresi = 40;
    
    int bipFrekans;
    if (mesafe >= 100) bipFrekans = 800;
    else if (mesafe >= 50) bipFrekans = 1000;
    else if (mesafe >= 20) bipFrekans = 1200;
    else bipFrekans = 1500;
    
    tone(BUZZER_PIN, bipFrekans, bipSuresi);
    sonBip = simdi;
    
    unsigned long bipBitis = simdi + bipSuresi;
    while (millis() < bipBitis) {
      // Bekle
    }
    beepCalisiyor = false;
  }
}

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  Serial.println("TOPRAK PRO MAX V5.5 baslatiliyor...");
  Serial.println("Servo sinirlari duzeltildi, tarama gelistirildi");
  
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
  
  for (int i = 0; i < MESAFE_BUFFER_SIZE; i++) {
    mesafeBuffer[i] = 100.0;
  }
  
  // Servo başlatma
  taramaServo.setPeriodHertz(50);
  taramaServo.attach(SERVO_PIN, 500, 2500);
  
  Serial.println("Servo sistemi test ediliyor...");
  
  // Servo testi
  servoAci = SERVO_HOME_POS;
  taramaServo.write(servoAci);
  delay(1000);
  
  Serial.println("Sag sinira gidiliyor...");
  for (int i = servoAci; i >= SERVO_MIN_LIMIT; i -= 5) {
    taramaServo.write(i);
    delay(100);
    Serial.print("Aci: ");
    Serial.println(i);
  }
  delay(500);
  
  Serial.println("Sol sinira gidiliyor...");
  for (int i = SERVO_MIN_LIMIT; i <= SERVO_MAX_LIMIT; i += 5) {
    taramaServo.write(i);
    delay(100);
    Serial.print("Aci: ");
    Serial.println(i);
  }
  delay(500);
  
  Serial.println("Orta noktaya donuluyor...");
  servoAci = SERVO_HOME_POS;
  taramaServo.write(servoAci);
  delay(500);
  
  hedefAci = SERVO_HOME_POS;
  
  Serial.println("Servo testi tamamlandi");
  
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED hatasi!");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
  }
  
  baslangic();
  
  Serial.println("HC-SR04 testi yapiliyor...");
  for (int i = 0; i < 5; i++) {
    float testMesafe = hcSr04Oku();
    Serial.print("Test ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(testMesafe);
    Serial.println(" cm");
    delay(100);
  }
  
  Serial.println("TOPRAK PRO MAX V5.5 - Sistem hazir!");
}

void baslangic() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPRAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("PRO MAX V5.5");
  display.display();
  
  delay(1000);
}

// =========== ANA DÖNGÜ ===========
void loop() {
  unsigned long simdi = millis();
  
  if (simdi - sonOkuma > 50) {
    sensorOku();
    uyariKontrol();
    sonOkuma = simdi;
  }
  
  butonOku();
  butonIsle();
  
  if (!manuelServoMode) {
    joyKontrol();
  }
  
  if (ayar.servoAktif) {
    if (manuelServoMode) {
      joyServoKontrol();
    } else if (taramaAktif) {
      servoKontrol();
    }
  }
  
  ledKontrol(simdi);
  buzzerKontrol(simdi);
  
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    ekranCiz();
  }
  
  if (simdi - sonGozKirpma > 3000 + random(0, 2000)) {
    gozAcik = !gozAcik;
    sonGozKirpma = simdi;
  }
  
  if (simdi - sonEtkilesim > 20000 && !uykuModu && !guvenlikAktif) {
    uykuModu = true;
    robotIfade = 4;
  } else if (simdi - sonEtkilesim < 20000 && uykuModu) {
    uykuModu = false;
    robotIfade = 0;
  }
  
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
        }
        break;
        
      case MENU_TARAMA:
        taramaAktif = !taramaAktif;
        if (taramaAktif) {
          Serial.println("Tarama AKTIF");
        } else {
          Serial.println("Tarama PASIF");
        }
        break;
        
      case MENU_SERVO_MANUEL:
        servoAci = SERVO_HOME_POS;
        taramaServo.write(servoAci);
        Serial.println("Servo orta noktaya getirildi");
        break;
        
      case MENU_GUVENLIK:
        guvenlikAktif = !guvenlikAktif;
        if (guvenlikAktif) {
          Serial.println("Guvenlik sistemi AKTIF");
          bipSesi(3);
        } else {
          alarmCalisiyor = false;
          noTone(BUZZER_PIN);
          Serial.println("Guvenlik sistemi PASIF");
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
        if (!ayar.ses) {
          noTone(BUZZER_PIN);
          beepCalisiyor = false;
        }
        Serial.print("Ses durumu: ");
        Serial.println(ayar.ses ? "ACIK" : "KAPALI");
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
      
    case MENU_AYAR_SERVO:
      if (ayarSecim == 0) {
        ayar.servoSpeed += yon * 1;
        ayar.servoSpeed = constrain(ayar.servoSpeed, 5, 50);
        taramaHizi = ayar.servoSpeed;
      } else if (ayarSecim == 1) {
        ayar.servoMinAci += yon * 5;
        ayar.servoMinAci = constrain(ayar.servoMinAci, 0, SERVO_MAX_LIMIT-10);
      } else if (ayarSecim == 2) {
        ayar.servoMaxAci += yon * 5;
        ayar.servoMaxAci = constrain(ayar.servoMaxAci, SERVO_MIN_LIMIT+10, 180);
      }
      break;
      
    case MENU_AYAR_LED:
      if (ayarSecim == 0) {
        ledMod += yon;
        if (ledMod > 3) ledMod = 0;
        if (ledMod < 0) ledMod = 3;
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
    "Guvenlik Sistemi"
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
  display.print("Sensör:");
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
  display.print("Mesafe: ");
  display.print(mesafe, 0);
  display.print(" cm");
  
  display.setCursor(5, 55);
  display.print("Hareket Sayisi: ");
  display.print(hareketSayisi);
}

// =========== GELİŞMİŞ TARAMA EKRANI ===========
void taramaEkranCiz() {
  int merkezX = 64;
  int merkezY = 35;
  int yaricap = 25;
  
  // Radar çemberleri
  display.drawCircle(merkezX, merkezY, yaricap, SSD1306_WHITE);
  display.drawCircle(merkezX, merkezY, yaricap/2, SSD1306_WHITE);
  
  // Servo yön çizgisi
  float radyan = radians(servoAci);
  int bitisX = merkezX + sin(radyan) * yaricap;
  int bitisY = merkezY - cos(radyan) * yaricap;
  display.drawLine(merkezX, merkezY, bitisX, bitisY, SSD1306_WHITE);
  
  // Tarama noktaları (geliştirilmiş)
  for (int i = 0; i < 37; i++) {
    float mesafeDegeri = taramaHaritasi[i];
    if (mesafeDegeri > 2.0 && mesafeDegeri < 200.0) {
      float aci = radians(i * 10);
      
      // Mesafeye göre boyut ve parlaklık
      int noktaBoyutu = 1;
      if (mesafeDegeri < 50) noktaBoyutu = 2;
      if (mesafeDegeri < 20) noktaBoyutu = 3;
      
      // Mesafe haritada göster
      float uzaklik = map(mesafeDegeri, 0, 200, 0, yaricap);
      int x = merkezX + sin(aci) * uzaklik;
      int y = merkezY - cos(aci) * uzaklik;
      
      // Renk kodlaması: kırmızı (yakın) - sarı (orta) - yeşil (uzak)
      if (mesafeDegeri < 30) {
        display.fillCircle(x, y, noktaBoyutu, SSD1306_WHITE);
      } else if (mesafeDegeri < 100) {
        display.drawCircle(x, y, noktaBoyutu, SSD1306_WHITE);
      } else {
        display.drawPixel(x, y, SSD1306_WHITE);
      }
    }
  }
  
  // Bilgi satırı
  display.setCursor(5, 55);
  display.print("A:");
  display.print(servoAci);
  display.print("° M:");
  display.print(mesafe, 0);
  display.print("cm");
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
  display.print("JOY Sol: SOLA");
  display.setCursor(5, 60);
  display.print("JOY Sag: SAGA");
}

// =========== AYAR EKRANLARI ===========
void ayarMesafeCiz() {
  if (ayarModu) {
    display.setCursor(5, 15);
    if (ayarSecim == 0) {
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
  display.print("TOPRAK PRO MAX V5.5");
  display.setCursor(5, 20);
  display.print("Servo: ");
  display.print(servoAci);
  display.print("°");
  display.setCursor(5, 30);
  display.print("Mesafe: ");
  display.print(mesafe, 1);
  display.print("cm");
  display.setCursor(5, 40);
  display.print("LED Mod: ");
  display.print(ledMod);
  display.print(" Hareket: ");
  display.print(hareket ? "VAR" : "YOK");
  display.setCursor(5, 50);
  display.print("Gaz: ");
  display.print(gaz, 0);
  display.print("ppm");
  display.setCursor(5, 60);
  display.print("D18: Ana Menu");
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
