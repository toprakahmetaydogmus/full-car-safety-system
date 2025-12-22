/****************************************************
 * TOPRAK - Akıllı Araba Asistanı PRO MAX V5.2
 * GELİŞTİRİLMİŞ SİSTEM
 * 
 * YENİ ÖZELLİKLER:
 * 1. Yaklaştıkça LED yanıp sönme hızı ARTIYOR
 * 2. Yaklaştıkça Buzzer bip sıklığı ARTIYOR
 * 3. Servo yönleri TERSİNE çevrildi
 * 4. Servo sağdan başlıyor, sola gitmeye çalışıyor
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

// =========== DEĞİŞKENLER ===========
// Sensörler
float mesafe = 0;
float gaz = 0;

// HC-SR04
float mesafeBuffer[10] = {0};
int bufferIndex = 0;
unsigned long sonMesafeOkuma = 0;
const int MESAFE_OKUMA_ARALIGI = 80;

// PIR
bool hareket = false;
unsigned long sonHareketZamani = 0;
int hareketSayisi = 0;
bool hareketTespitEdildi = false;
int hareketSikligi = 0;

// Butonlar
bool joyBtn = false, lastJoyBtn = false;
bool geriBtn = false, lastGeriBtn = false;

// Menü Sistemi
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
#define MENU_OTONOM 14
#define MENU_AYAR_PIR 15

int aktifMenu = MENU_ANA;
int menuSecim = 0;
bool ayarModu = false;
int ayarSecim = 0;
bool manuelServoMode = false;

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

// Servo Sistemi - TERS YÖN
int servoAci = 170; // SAĞDAN BAŞLA
int hedefAci = 170;
bool taramaYonu = false; // false = SOLA, true = SAĞA (TERS)
bool taramaAktif = true;
int taramaHizi = 5;
float taramaHaritasi[37] = {0};
unsigned long sonServoHareket = 0;
bool alarmModu = false;
int alarmServoHizi = 10;

// LED Sistemi - YAKLAŞTIKÇA HIZLANAN
int ledParlaklik = 200;
int ledMod = 2; // Mesafe feedback modu
bool ledDurum = true;
unsigned long sonLedDegisim = 0;
int ledBlinkSpeed = 1000; // Başlangıç hızı

// Buzzer Sistemi - YAKLAŞTIKÇA HIZLANAN
unsigned long sonBip = 0;
int bipInterval = 1500; // Başlangıç aralığı
bool bipDurum = false;

// Güvenlik Sistemi
bool guvenlikAktif = false;
bool alarmCalisiyor = false;
unsigned long alarmBaslangic = 0;
int alarmSeviye = 0;

// Joystick Servo Kontrol - TERS YÖN
int joyServoSpeed = 15;
unsigned long sonJoyServoHareket = 0;

// Ayarlar
struct {
  int mesafeDikkat = 50;
  int mesafeTehlike = 20;
  int gazEsik = 800;
  int hareketEsikSure = 3000;
  bool ses = true;
  
  bool servoAktif = true;
  int servoSpeed = 5;
  int servoMinAci = 10;
  int servoMaxAci = 170;
  
  bool ledAktif = true;
  int ledMode = 2;
  int ledHiz = 500;
  int ledBright = 200;
  
  bool guvenlikOtomatik = false;
  int guvenlikHassasiyet = 2;
  int alarmSuresi = 10000;
  
  int pirHassasiyet = 2;
  bool pirFiltre = true;
  
  bool robotAnimasyon = true;
  
  int joyServoHiz = 15;
} ayar;

// Zamanlayıcılar
unsigned long sonOkuma = 0;
unsigned long sonEtkilesim = 0;
bool uykuModu = false;

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  
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
  
  // Servo - Ters yön için
  taramaServo.attach(SERVO_PIN, 500, 2500);
  Serial.println("Servo ters yon baslatiliyor...");
  
  // Ters yön testi
  taramaServo.write(170); // SAĞ
  delay(200);
  taramaServo.write(10);  // SOL
  delay(200);
  taramaServo.write(90);  // ORTA
  delay(200);
  taramaServo.write(170); // SAĞDAN BAŞLA
  delay(200);
  
  servoAci = 170;
  hedefAci = 170;
  
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED hatasi!");
    while(1);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  baslangic();
  Serial.println("TOPRAK PRO MAX V5.2 - Sistem hazir!");
  Serial.println("Servo: Ters yon aktif");
  Serial.println("LED/Buzzer: Mesafeye gore hizlanan");
}

void baslangic() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPRAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("PRO MAX V5.2");
  display.display();
  
  // Hızlanan LED testi
  if(ayar.ledAktif) {
    Serial.println("LED: Hizlanan yanip sonme testi");
    
    for(int speed = 1000; speed > 50; speed -= 100) {
      analogWrite(LED_PIN, 255);
      delay(speed/2);
      analogWrite(LED_PIN, 0);
      delay(speed/2);
    }
    
    analogWrite(LED_PIN, ledParlaklik);
  }
  
  // Hızlanan buzzer testi
  if(ayar.ses) {
    Serial.println("Buzzer: Hizlanan bip testi");
    
    for(int speed = 800; speed > 100; speed -= 100) {
      tone(BUZZER_PIN, 1000, 80);
      delay(speed);
    }
    noTone(BUZZER_PIN);
  }
  
  delay(1000);
}

// =========== HC-SR04 OKUMA ===========
float hcSr04Oku() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long sure = pulseIn(ECHO_PIN, HIGH, 30000);
  if(sure == 0) return 400.0;
  
  float mesafeCm = sure * 0.034 / 2;
  if(mesafeCm < 2.0 || mesafeCm > 400.0) return 400.0;
  
  return mesafeCm;
}

float hcSr04OkuFiltreli() {
  static float sonGecerliMesafe = 100.0;
  
  // 3 okuma al
  float okumalar[3];
  for(int i = 0; i < 3; i++) {
    okumalar[i] = hcSr04Oku();
    delay(5);
  }
  
  // Medyan filtresi
  float temp;
  if(okumalar[0] > okumalar[1]) {
    temp = okumalar[0];
    okumalar[0] = okumalar[1];
    okumalar[1] = temp;
  }
  if(okumalar[1] > okumalar[2]) {
    temp = okumalar[1];
    okumalar[1] = okumalar[2];
    okumalar[2] = temp;
  }
  if(okumalar[0] > okumalar[1]) {
    temp = okumalar[0];
    okumalar[0] = okumalar[1];
    okumalar[1] = temp;
  }
  
  float medyan = okumalar[1];
  
  // Hareketli ortalama
  mesafeBuffer[bufferIndex] = medyan;
  bufferIndex = (bufferIndex + 1) % 10;
  
  float toplam = 0;
  int sayac = 0;
  for(int i = 0; i < 10; i++) {
    if(mesafeBuffer[i] > 2.0 && mesafeBuffer[i] < 400.0) {
      toplam += mesafeBuffer[i];
      sayac++;
    }
  }
  
  if(sayac > 0) {
    float ortalama = toplam / sayac;
    
    // Ani değişimleri filtrele
    if(abs(ortalama - sonGecerliMesafe) < 50.0 || sonGecerliMesafe == 100.0) {
      sonGecerliMesafe = ortalama;
    }
    
    return sonGecerliMesafe;
  }
  
  return sonGecerliMesafe;
}

// =========== SENSÖR OKUMA ===========
void sensorOku() {
  unsigned long simdi = millis();
  
  if(simdi - sonMesafeOkuma >= MESAFE_OKUMA_ARALIGI) {
    mesafe = hcSr04OkuFiltreli();
    sonMesafeOkuma = simdi;
    
    // Mesafe değişti, LED ve buzzer hızını güncelle
    updateBlinkSpeed();
  }
  
  // Gaz okuma
  int gazDeger = analogRead(GAS_PIN);
  gaz = map(gazDeger, 0, 4095, 0, 2000);
  
  // Debug
  static unsigned long sonDebug = 0;
  if(simdi - sonDebug > 1000) {
    Serial.print("Mesafe: ");
    Serial.print(mesafe);
    Serial.print("cm | LED Hiz: ");
    Serial.print(ledBlinkSpeed);
    Serial.print("ms | Bip Hiz: ");
    Serial.print(bipInterval);
    Serial.println("ms");
    sonDebug = simdi;
  }
}

// =========== LED VE BUZZER HIZ GÜNCELLEME ===========
void updateBlinkSpeed() {
  // Mesafeye göre LED yanıp sönme hızı
  if(mesafe >= 200) {
    ledBlinkSpeed = 2000;  // Çok uzak - çok yavaş
    bipInterval = 2500;    // Çok uzak - çok seyrek
  } else if(mesafe >= 150) {
    ledBlinkSpeed = 1500;
    bipInterval = 2000;
  } else if(mesafe >= 100) {
    ledBlinkSpeed = 1000;
    bipInterval = 1500;
  } else if(mesafe >= 70) {
    ledBlinkSpeed = 700;
    bipInterval = 1000;
  } else if(mesafe >= 50) {
    ledBlinkSpeed = 500;
    bipInterval = 700;
  } else if(mesafe >= 30) {
    ledBlinkSpeed = 300;
    bipInterval = 500;
  } else if(mesafe >= 20) {
    ledBlinkSpeed = 200;
    bipInterval = 300;
  } else if(mesafe >= 10) {
    ledBlinkSpeed = 100;
    bipInterval = 200;
  } else {
    ledBlinkSpeed = 50;    // Çok yakın - çok hızlı
    bipInterval = 100;     // Çok yakın - çok sık
  }
}

// =========== UYARI KONTROL ===========
void uyariKontrol() {
  bool oncekiUyari = uyariVar;
  
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
  } else {
    robotIfade = 0;
  }
  
  if(uykuModu) {
    robotIfade = 4;
  }
  
  if((oncekiUyari != uyariVar)) {
    if(!uyariVar && ayar.ses) {
      noTone(BUZZER_PIN);
      beepCalisiyor = false;
    }
  }
  
  digitalWrite(SYS_LED, uyariVar);
}

// =========== SERVO KONTROLÜ - TERS YÖN ===========
void servoKontrol() {
  unsigned long simdi = millis();
  
  if(servoAci != hedefAci) {
    // Hızlı hareket
    if(servoAci < hedefAci) servoAci++;
    else servoAci--;
    
    taramaServo.write(servoAci);
  } else {
    // TERS YÖN: false = SOLA, true = SAĞA
    if(!taramaYonu) { // SOLA gidiyor
      hedefAci -= 10;
      if(hedefAci < ayar.servoMinAci) {
        hedefAci = ayar.servoMinAci;
        taramaYonu = true; // SAĞA dön
      }
    } else { // SAĞA gidiyor
      hedefAci += 10;
      if(hedefAci > ayar.servoMaxAci) {
        hedefAci = ayar.servoMaxAci;
        taramaYonu = false; // SOLA dön
      }
    }
    
    // Tarama haritasını güncelle
    int taramaIndex = map(servoAci, 0, 180, 0, 36);
    taramaIndex = constrain(taramaIndex, 0, 36);
    taramaHaritasi[taramaIndex] = mesafe;
  }
}

// =========== JOYSTICK İLE SERVO KONTROL - TERS YÖN ===========
void joyServoKontrol() {
  unsigned long simdi = millis();
  
  if(simdi - sonJoyServoHareket < joyServoSpeed) {
    return;
  }
  
  int joyX = analogRead(JOY_X);
  
  // TERS YÖN: Sol joystick SAĞA, Sağ joystick SOLA
  if(joyX < 1000) { // JOYSTICK SOL - SERVO SAĞA
    servoAci += 5;
    if(servoAci > ayar.servoMaxAci) servoAci = ayar.servoMaxAci;
    taramaServo.write(servoAci);
    sonJoyServoHareket = simdi;
  } 
  else if(joyX > 3000) { // JOYSTICK SAĞ - SERVO SOLA
    servoAci -= 5;
    if(servoAci < ayar.servoMinAci) servoAci = ayar.servoMinAci;
    taramaServo.write(servoAci);
    sonJoyServoHareket = simdi;
  }
  
  // Joystick butonu ile merkeze dön (90 derece)
  if(digitalRead(JOY_BTN) == LOW) {
    servoAci = 90;
    taramaServo.write(servoAci);
    delay(100);
  }
}

// =========== LED KONTROLÜ - HIZLANAN YANIP SÖNME ===========
void ledKontrol(unsigned long simdi) {
  if(!ayar.ledAktif) {
    analogWrite(LED_PIN, 0);
    return;
  }
  
  switch(ledMod) {
    case 0: // Sabit
      analogWrite(LED_PIN, ledParlaklik);
      break;
      
    case 1: // Sabit hızda yanıp sönme
      if(simdi - sonLedDegisim > ayar.ledHiz) {
        ledDurum = !ledDurum;
        analogWrite(LED_PIN, ledDurum ? ledParlaklik : 0);
        sonLedDegisim = simdi;
      }
      break;
      
    case 2: // Mesafeye göre hızlanan yanıp sönme
      if(simdi - sonLedDegisim > ledBlinkSpeed) {
        ledDurum = !ledDurum;
        
        if(ledDurum) {
          // AÇIK - Parlaklık mesafeye göre
          int parlaklik;
          if(mesafe >= 100) parlaklik = 50;
          else if(mesafe >= 50) parlaklik = map(mesafe, 50, 100, 100, 50);
          else if(mesafe >= 20) parlaklik = map(mesafe, 20, 50, 180, 100);
          else parlaklik = 255;
          
          analogWrite(LED_PIN, parlaklik);
        } else {
          // KAPALI
          analogWrite(LED_PIN, 0);
        }
        
        sonLedDegisim = simdi;
      }
      break;
      
    case 3: // Hareket feedback
      if(hareketTespitEdildi) {
        analogWrite(LED_PIN, ledParlaklik);
      } else {
        analogWrite(LED_PIN, 0);
      }
      break;
  }
}

// =========== BUZZER KONTROLÜ - HIZLANAN BIP ===========
void buzzerKontrol(unsigned long simdi) {
  if(!ayar.ses || !uyariVar || beepCalisiyor) return;
  
  // Uyarı 3 saniyeden eskiyse bip sesini durdur
  if(simdi - sonUyariZamani > 3000) {
    noTone(BUZZER_PIN);
    return;
  }
  
  if(simdi - sonBip > bipInterval) {
    beepCalisiyor = true;
    
    // Bip süresi de mesafeye göre değişsin
    int bipSuresi;
    if(mesafe >= 100) bipSuresi = 100;
    else if(mesafe >= 50) bipSuresi = 80;
    else if(mesafe >= 20) bipSuresi = 60;
    else bipSuresi = 40;
    
    // Frekans da mesafeye göre değişsin
    int bipFrekans;
    if(mesafe >= 100) bipFrekans = 800;
    else if(mesafe >= 50) bipFrekans = 1000;
    else if(mesafe >= 20) bipFrekans = 1200;
    else bipFrekans = 1500;
    
    tone(BUZZER_PIN, bipFrekans, bipSuresi);
    sonBip = simdi;
    
    // Bip bittikten sonra flag'i sıfırla
    unsigned long bipBitis = simdi + bipSuresi;
    while(millis() < bipBitis) {
      // Bekle
    }
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
    } else if(taramaAktif && simdi - sonServoHareket > taramaHizi) {
      servoKontrol();
      sonServoHareket = simdi;
    }
  }
  
  // LED kontrol
  ledKontrol(simdi);
  
  // Buzzer kontrol
  buzzerKontrol(simdi);
  
  // Ekran
  ekranCiz();
  
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
  
  // D18 - ANA MENÜ
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
          case 4: aktifMenu = MENU_SERVO_MANUEL; manuelServoMode = true; break;
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
    "Servo Manuel Kontrol",
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
  
  // LED/Buzzer hızı göster
  display.setCursor(5, 55);
  display.print("LED:");
  display.print(ledBlinkSpeed);
  display.print("ms BIP:");
  display.print(bipInterval);
  display.print("ms");
}

void gazEkranCiz() {
  display.setTextSize(3);
  display.setCursor(15, 15);
  display.print(gaz, 0);
  
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("ppm");
  
  display.setCursor(5, 55);
  if(gaz < 500) display.print("HAVA KALITESI: IYI");
  else if(gaz < 1000) display.print("HAVA KALITESI: ORTA");
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
  display.print("LED Hiz: ");
  display.print(ledBlinkSpeed);
  display.print(" ms");
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
  display.print("° H:");
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
  display.print("JOY Sol: SAGA");
  display.setCursor(5, 60);
  display.print("JOY Sag: SOLA");
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
  display.print("TOPRAK PRO MAX V5.2");
  display.setCursor(5, 20);
  display.print("Servo: ");
  display.print(servoAci);
  display.print("° Ters Yon");
  display.setCursor(5, 30);
  display.print("Mesafe: ");
  display.print(mesafe, 1);
  display.print("cm");
  display.setCursor(5, 40);
  display.print("LED Hiz: ");
  display.print(ledBlinkSpeed);
  display.print("ms");
  display.setCursor(5, 50);
  display.print("BIP Hiz: ");
  display.print(bipInterval);
  display.print("ms");
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
  display.print("PASIF");
  
  display.setTextSize(1);
  display.setCursor(5, 45);
  display.print("Bu mod");
  display.setCursor(5, 55);
  display.print("geliştiriliyor");
}

void ayarPirCiz() {
  display.setCursor(10, 15);
  display.print("PIR Kalibrasyon");
  
  display.setCursor(10, 30);
  display.print("Hazir");
  display.setCursor(10, 40);
  display.print("Buton: Baslat");
}

void robotCiz(int x, int y) {
  display.drawCircle(x, y, 6, SSD1306_WHITE);
  
  if(gozAcik) {
    if(robotIfade == 0) {
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawPixel(x, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 2) {
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
