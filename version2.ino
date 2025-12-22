/****************************************************
 * TOPRAK - Akıllı Araba Asistanı PRO MAX
 * GELİŞTİRİLMİŞ SON SÜRÜM
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
#include <DHT.h>
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
#define DHT_PIN 14
#define DHT_TYPE DHT11
#define BUZZER_PIN 15
#define SYS_LED 2
#define SERVO_PIN 26
#define LED_PIN 25

// =========== NESNELER ===========
Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(DHT_PIN, DHT_TYPE);
Servo taramaServo;

// =========== DEĞİŞKENLER ===========
// Sensörler
float mesafe = 0;
float gaz = 0;
float sicaklik = 0;
float nem = 0;

// Butonlar
bool joyBtn = false, lastJoyBtn = false;
bool geriBtn = false, lastGeriBtn = false;

// Menü Sistemi
#define MENU_ANA 0
#define MENU_MESAFE 1
#define MENU_GAZ 2
#define MENU_SICAKLIK 3
#define MENU_TARAMA 4
#define MENU_AYAR_MESAFE 5
#define MENU_AYAR_GAZ 6
#define MENU_AYAR_SICAKLIK 7
#define MENU_AYAR_SES 8
#define MENU_AYAR_SERVO 9
#define MENU_AYAR_LED 10
#define MENU_SISTEM 11

int aktifMenu = MENU_ANA;
int menuSecim = 0;
bool ayarModu = false;
int ayarSecim = 0;

// Ana Menü Kaydırma
int menuBaslangic = 0;
const int MAX_GORUNEN_OGE = 6; // Ekranda görünen maksimum öğe sayısı

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0; // 0: yok, 1: dikkat, 2: tehlike

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0; // 0: normal, 1: dikkat, 2: tehlike, 3: mutlu, 4: uyku

// Servo Sistemi
int servoAci = 90;
int hedefAci = 90;
bool taramaYonu = true;
bool taramaAktif = true;
int taramaHizi = 20;
float taramaHaritasi[36] = {0};
unsigned long sonServoHareket = 0;

// LED Sistemi
int ledParlaklik = 150;
int ledMod = 0; // 0: Sabit, 1: Yanıp Sönme, 2: Mesafe Feedback
bool ledDurum = true;
unsigned long sonLedDegisim = 0;

// Ayarlar
struct {
  int mesafeDikkat = 50;   // cm
  int mesafeTehlike = 20;  // cm
  int gazEsik = 800;       // ppm
  int sicaklikEsik = 35;   // °C
  bool ses = true;
  
  // Yeni ayarlar
  bool servoAktif = true;
  int servoSpeed = 20;
  int servoMinAci = 20;
  int servoMaxAci = 160;
  
  bool ledAktif = true;
  int ledMode = 0;
  int ledHiz = 500;
  int ledBright = 150;
  
  bool robotAnimasyon = true;
} ayar;

// Zamanlayıcılar
unsigned long sonOkuma = 0;
unsigned long sonBip = 0;
unsigned long sonEtkilesim = 0;
bool uykuModu = false;

// =========== FONKSİYON PROTOTİPLERİ ===========
void sensorOku();
void uyariKontrol();
void butonOku();
void butonIsle();
void joyKontrol();
void servoKontrol();
void ledKontrol(unsigned long simdi);
void ekranCiz();
void baslikCiz();
void icerikCiz();
void anaMenuCiz();
void mesafeEkranCiz();
void gazEkranCiz();
void sicaklikEkranCiz();
void taramaEkranCiz();
void ayarMesafeCiz();
void ayarGazCiz();
void ayarSicaklikCiz();
void ayarSesCiz();
void ayarServoCiz();
void ayarLedCiz();
void sistemEkranCiz();
void robotCiz(int x, int y);
void altBilgiCiz();
void bipSesi(int tip);
void sesLED();
void sistemTest();
void acilDurum();
void ayarlariKaydet();
void ayarlariYukle();
void yukleniyorEkrani(String mesaj);
void otoKalibrasyon();
void mesafeGrafik();
void ayarDegistir(int yon);

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  
  // Pinler
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(GERI_BUTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SYS_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
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
  
  // DHT
  dht.begin();
  
  // Başlangıç
  baslangic();
  Serial.println("TOPRAK PRO MAX - Sistem hazir!");
}

void baslangic() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPRAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("PRO MAX");
  display.display();
  
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

// =========== ANA DÖNGÜ ===========
void loop() {
  unsigned long simdi = millis();
  
  // 1. Sensör okuma (400ms)
  if(simdi - sonOkuma > 400) {
    sensorOku();
    uyariKontrol();
    sonOkuma = simdi;
  }
  
  // 2. Buton kontrolü
  butonOku();
  butonIsle();
  
  // 3. Joystick kontrolü
  joyKontrol();
  
  // 4. Servo kontrolü (otomatik tarama)
  if(ayar.servoAktif && taramaAktif && simdi - sonServoHareket > taramaHizi) {
    servoKontrol();
    sonServoHareket = simdi;
  }
  
  // 5. LED kontrolü
  if(ayar.ledAktif && simdi - sonLedDegisim > ayar.ledHiz) {
    ledKontrol(simdi);
    sonLedDegisim = simdi;
  }
  
  // 6. Ekran güncelle
  ekranCiz();
  
  // 7. Ses ve LED uyarıları
  sesLED();
  
  // 8. Animasyon
  if(ayar.robotAnimasyon && simdi - sonGozKirpma > 3000 + random(0, 2000)) {
    gozAcik = !gozAcik;
    sonGozKirpma = simdi;
  }
  
  // 9. Uyku modu kontrolü
  if(simdi - sonEtkilesim > 20000 && !uykuModu) {
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
  // Mesafe (ORİJİNAL ÖLÇÜM SİSTEMİ)
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
  
  // Sıcaklık/Nem (2 saniyede bir)
  static unsigned long sonDHT = 0;
  if(millis() - sonDHT > 2000) {
    sicaklik = dht.readTemperature();
    nem = dht.readHumidity();
    if(isnan(sicaklik)) sicaklik = 0;
    if(isnan(nem)) nem = 0;
    sonDHT = millis();
  }
  
  // Debug
  static unsigned long sonSerial = 0;
  if(millis() - sonSerial > 1000) {
    Serial.print("M:");
    Serial.print(mesafe);
    Serial.print("cm G:");
    Serial.print(gaz);
    Serial.print("ppm S:");
    Serial.print(sicaklik);
    Serial.println("C");
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
  } else if(sicaklik > ayar.sicaklikEsik) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
  } else {
    robotIfade = 0;
  }
  
  // Uyku modundaysa
  if(uykuModu) {
    robotIfade = 4;
  }
  
  // Eğer uyarı durumu değiştiyse
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
    
    // Tarama haritasını güncelle
    int taramaIndex = map(servoAci, 0, 180, 0, 35);
    taramaHaritasi[taramaIndex] = mesafe;
  }
}

// =========== LED KONTROLÜ ===========
void ledKontrol(unsigned long simdi) {
  switch(ledMod) {
    case 0: // Sabit
      analogWrite(LED_PIN, ledParlaklik);
      break;
      
    case 1: // Yanıp sönme
      ledDurum = !ledDurum;
      analogWrite(LED_PIN, ledDurum ? ledParlaklik : 0);
      break;
      
    case 2: // Mesafe feedback
      int blinkSpeed;
      if(mesafe < 30) blinkSpeed = 100;
      else if(mesafe < 60) blinkSpeed = 200;
      else if(mesafe < 100) blinkSpeed = 400;
      else blinkSpeed = 800;
      
      if(simdi - sonLedDegisim > blinkSpeed) {
        ledDurum = !ledDurum;
        analogWrite(LED_PIN, ledDurum ? ledParlaklik : 0);
        sonLedDegisim = simdi;
      }
      break;
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
    robotIfade = 3; // Mutlu ifade
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
        // Ana menüden seçime git
        switch(menuSecim) {
          case 0: aktifMenu = MENU_MESAFE; break;
          case 1: aktifMenu = MENU_GAZ; break;
          case 2: aktifMenu = MENU_SICAKLIK; break;
          case 3: aktifMenu = MENU_TARAMA; break;
          case 4: aktifMenu = MENU_AYAR_MESAFE; break;
          case 5: aktifMenu = MENU_AYAR_GAZ; break;
          case 6: aktifMenu = MENU_AYAR_SICAKLIK; break;
          case 7: aktifMenu = MENU_AYAR_SES; break;
          case 8: aktifMenu = MENU_AYAR_SERVO; break;
          case 9: aktifMenu = MENU_AYAR_LED; break;
          case 10: aktifMenu = MENU_SISTEM; break;
        }
        break;
        
      case MENU_TARAMA:
        taramaAktif = !taramaAktif;
        break;
        
      case MENU_AYAR_MESAFE:
      case MENU_AYAR_GAZ:
      case MENU_AYAR_SICAKLIK:
      case MENU_AYAR_SERVO:
      case MENU_AYAR_LED:
        // Ayar ekranlarında ayar modunu aç/kapa
        ayarModu = !ayarModu;
        ayarSecim = 0;
        break;
        
      case MENU_AYAR_SES:
        // Ses ayarını değiştir
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
  
  // YUKARI/AŞAĞI - Menü gezinme
  if(joyY < 1000) { // YUKARI
    hareketVar = true;
    if(aktifMenu == MENU_ANA) {
      menuSecim--;
      if(menuSecim < 0) menuSecim = 10;
      
      // Kaydırma kontrolü
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
      if(menuSecim > 10) menuSecim = 0;
      
      // Kaydırma kontrolü
      if(menuSecim < menuBaslangic) {
        menuBaslangic = menuSecim;
      } else if(menuSecim >= menuBaslangic + MAX_GORUNEN_OGE) {
        menuBaslangic = menuSecim - MAX_GORUNEN_OGE + 1;
      }
    } else if(ayarModu) {
      ayarSecim++;
    }
  }
  
  // SAĞ/SOL - Değer değiştirme (ayar modunda)
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
      
    case MENU_AYAR_SICAKLIK:
      ayar.sicaklikEsik += yon * 5;
      ayar.sicaklikEsik = constrain(ayar.sicaklikEsik, 20, 50);
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
        ayar.ledMode = constrain(ayar.ledMode, 0, 2);
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
  
  // Başlık çubuğu
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  baslikCiz();
  
  // Ana içerik
  icerikCiz();
  
  // Robot karakter (sağ alt köşe)
  robotCiz(110, 50);
  
  // Alt bilgi çubuğu
  altBilgiCiz();
  
  display.display();
}

void baslikCiz() {
  display.setCursor(40, 2);
  switch(aktifMenu) {
    case MENU_ANA: display.print("ANA MENU"); break;
    case MENU_MESAFE: display.print("MESAFE"); break;
    case MENU_GAZ: display.print("GAZ"); break;
    case MENU_SICAKLIK: display.print("SICAKLIK"); break;
    case MENU_TARAMA: display.print("TARAMA"); break;
    case MENU_AYAR_MESAFE: display.print("AYAR MESAFE"); break;
    case MENU_AYAR_GAZ: display.print("AYAR GAZ"); break;
    case MENU_AYAR_SICAKLIK: display.print("AYAR SICAKLIK"); break;
    case MENU_AYAR_SES: display.print("AYAR SES"); break;
    case MENU_AYAR_SERVO: display.print("AYAR SERVO"); break;
    case MENU_AYAR_LED: display.print("AYAR LED"); break;
    case MENU_SISTEM: display.print("SISTEM"); break;
  }
}

void icerikCiz() {
  switch(aktifMenu) {
    case MENU_ANA:
      anaMenuCiz();
      break;
    case MENU_MESAFE:
      mesafeEkranCiz();
      break;
    case MENU_GAZ:
      gazEkranCiz();
      break;
    case MENU_SICAKLIK:
      sicaklikEkranCiz();
      break;
    case MENU_TARAMA:
      taramaEkranCiz();
      break;
    case MENU_AYAR_MESAFE:
      ayarMesafeCiz();
      break;
    case MENU_AYAR_GAZ:
      ayarGazCiz();
      break;
    case MENU_AYAR_SICAKLIK:
      ayarSicaklikCiz();
      break;
    case MENU_AYAR_SES:
      ayarSesCiz();
      break;
    case MENU_AYAR_SERVO:
      ayarServoCiz();
      break;
    case MENU_AYAR_LED:
      ayarLedCiz();
      break;
    case MENU_SISTEM:
      sistemEkranCiz();
      break;
  }
}

void anaMenuCiz() {
  const char* menuItems[] = {
    "Mesafe Goster",
    "Gaz Goster",
    "Sicaklik Goster",
    "Tarama Ekrani",
    "Mesafe Ayari",
    "Gaz Ayari",
    "Sicaklik Ayari",
    "Ses Ayari",
    "Servo Ayari",
    "LED Ayari",
    "Sistem Bilgisi"
  };
  
  // Görüntülenecek aralık
  int baslangic = menuBaslangic;
  int bitis = min(baslangic + MAX_GORUNEN_OGE, 11);
  
  for(int i = baslangic; i < bitis; i++) {
    int listIndex = i - baslangic;
    int y = 15 + listIndex * 9; // Satır aralığı
    
    if(i == menuSecim) {
      // Seçili öğe için kutu
      display.fillRect(2, y-1, 124, 9, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(4, y);
      display.print(">");
      display.setCursor(12, y);
      display.print(menuItems[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      // Normal öğe
      display.setCursor(12, y);
      display.print(menuItems[i]);
    }
  }
  
  // Kaydırma göstergesi (sadece gerekirse)
  if(11 > MAX_GORUNEN_OGE) {
    int scrollBarHeight = 40;
    int scrollBarY = 15;
    int scrollPos = map(menuBaslangic, 0, 11 - MAX_GORUNEN_OGE, scrollBarY, scrollBarY + scrollBarHeight - 5);
    
    display.drawRect(125, scrollBarY, 2, scrollBarHeight, SSD1306_WHITE);
    display.fillRect(125, scrollPos, 2, 5, SSD1306_WHITE);
  }
}

void mesafeEkranCiz() {
  // Büyük rakam
  display.setTextSize(3);
  display.setCursor(15, 15);
  if(mesafe >= 100) {
    display.print(">100");
  } else if(mesafe >= 10) {
    display.print(mesafe, 0);
  } else {
    display.print(mesafe, 1);
  }
  
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
  
  // Ayarlar
  display.setCursor(5, 60);
  display.print("D:");
  display.print(ayar.mesafeDikkat);
  display.print(" T:");
  display.print(ayar.mesafeTehlike);
}

void gazEkranCiz() {
  // Büyük rakam
  display.setTextSize(3);
  display.setCursor(15, 15);
  if(gaz >= 1000) {
    display.print(gaz, 0);
  } else {
    display.print(gaz, 0);
  }
  
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("ppm");
  
  // Hava kalitesi
  display.setCursor(5, 50);
  display.print("Kalite: ");
  
  if(gaz < 500) {
    display.print("IYI");
  } else if(gaz < 1000) {
    display.print("ORTA");
  } else {
    display.print("KOTU");
  }
  
  // Uyarı eşiği
  display.setCursor(5, 60);
  display.print("Esik: ");
  display.print(ayar.gazEsik);
  display.print("ppm");
}

void sicaklikEkranCiz() {
  // Büyük rakam
  display.setTextSize(3);
  display.setCursor(15, 15);
  display.print(sicaklik, 1);
  
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("C");
  
  display.setCursor(5, 50);
  display.print("Nem: ");
  display.print(nem, 0);
  display.print("%");
  
  display.setCursor(5, 60);
  display.print("Esik: ");
  display.print(ayar.sicaklikEsik);
  display.print("C");
}

void taramaEkranCiz() {
  // Radar ekranı
  int merkezX = 64;
  int merkezY = 35;
  int yaricap = 25;
  
  // Radar dairesi
  display.drawCircle(merkezX, merkezY, yaricap, SSD1306_WHITE);
  display.drawCircle(merkezX, merkezY, yaricap/2, SSD1306_WHITE);
  
  // Tarama çizgisi
  float radyan = radians(servoAci);
  int bitisX = merkezX + sin(radyan) * yaricap;
  int bitisY = merkezY - cos(radyan) * yaricap;
  display.drawLine(merkezX, merkezY, bitisX, bitisY, SSD1306_WHITE);
  
  // Taranan noktalar
  for(int i = 0; i < 36; i++) {
    if(taramaHaritasi[i] > 2 && taramaHaritasi[i] < 100) {
      float aci = radians(i * 10);
      float uzaklik = map(taramaHaritasi[i], 0, 100, 0, yaricap);
      int x = merkezX + sin(aci) * uzaklik;
      int y = merkezY - cos(aci) * uzaklik;
      display.fillCircle(x, y, 1, SSD1306_WHITE);
    }
  }
  
  // Bilgiler
  display.setCursor(5, 55);
  display.print("A:");
  display.print(servoAci);
  display.print(" ");
  display.print(taramaAktif ? "AKTIF" : "DURDU");
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
      display.setTextSize(1);
    } else {
      display.print("Tehlike:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.mesafeTehlike);
      display.print("cm");
      display.setTextSize(1);
    }
    
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

void ayarSicaklikCiz() {
  if(ayarModu) {
    display.setCursor(5, 15);
    display.print("Sicaklik Esigi:");
    display.setTextSize(2);
    display.setCursor(30, 30);
    display.print(ayar.sicaklikEsik);
    display.print("C");
    display.setTextSize(1);
    
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Mevcut: ");
    display.print(sicaklik, 1);
    display.print("C");
    
    display.setCursor(5, 25);
    display.print("Esik: ");
    display.print(ayar.sicaklikEsik);
    display.print("C");
    
    display.setCursor(5, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarSesCiz() {
  display.setCursor(10, 20);
  display.print("Ses Durumu:");
  
  display.setTextSize(2);
  display.setCursor(40, 35);
  if(ayar.ses) {
    display.print("ACIK");
  } else {
    display.print("KAPALI");
  }
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
      display.setTextSize(1);
    } else if(ayarSecim == 1) {
      display.print("Min Aci:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoMinAci);
      display.print("°");
      display.setTextSize(1);
    } else {
      display.print("Max Aci:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoMaxAci);
      display.print("°");
      display.setTextSize(1);
    }
    
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
      }
      display.setTextSize(1);
    } else if(ayarSecim == 1) {
      display.print("LED Hizi:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.ledHiz);
      display.print("ms");
      display.setTextSize(1);
    } else {
      display.print("Parlaklik:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.ledBright);
      display.setTextSize(1);
    }
    
    display.setCursor(5, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    display.setCursor(5, 15);
    display.print("Mod: ");
    switch(ayar.ledMode) {
      case 0: display.print("SABIT"); break;
      case 1: display.print("YANIP SONME"); break;
      case 2: display.print("MESAFE"); break;
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
  display.print("TOPRAK PRO MAX");
  
  display.setCursor(5, 20);
  display.print("V3.1");
  
  display.setCursor(5, 30);
  display.print("Hafiza: ");
  display.print(ESP.getFreeHeap() / 1024.0, 1);
  display.print("KB");
  
  display.setCursor(5, 40);
  display.print("IP: 192.168.1.1");
  
  display.setCursor(5, 50);
  display.print("Uyku: ");
  display.print(uykuModu ? "AKTIF" : "PASIF");
  
  display.setCursor(5, 60);
  display.print("D18: Yeniden Baslat");
}

void robotCiz(int x, int y) {
  // Robot kafa - daha küçük
  display.drawCircle(x, y, 6, SSD1306_WHITE);
  
  // Gözler (ifadeye göre)
  if(gozAcik) {
    if(robotIfade == 0) { // Normal
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawPixel(x, y+2, SSD1306_WHITE); // Gülümseme
    }
    else if(robotIfade == 1) { // Dikkat
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawLine(x-2, y+2, x+2, y+2, SSD1306_WHITE); // Düz ağız
    }
    else if(robotIfade == 2) { // Tehlike
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawLine(x-2, y+2, x+2, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 3) { // Mutlu
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawLine(x-2, y+3, x+2, y+3, SSD1306_WHITE);
      display.drawPixel(x-1, y+2, SSD1306_WHITE);
      display.drawPixel(x+1, y+2, SSD1306_WHITE);
    }
    else if(robotIfade == 4) { // Uyku
      display.drawLine(x-3, y-1, x-1, y-1, SSD1306_WHITE);
      display.drawLine(x+1, y-1, x+3, y-1, SSD1306_WHITE);
      display.drawPixel(x, y+2, SSD1306_WHITE);
    }
  } else {
    // Göz kapalı (kırpma)
    display.drawLine(x-3, y-1, x-1, y-1, SSD1306_WHITE);
    display.drawLine(x+1, y-1, x+3, y-1, SSD1306_WHITE);
  }
  
  // Uyarı işareti (daha küçük)
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
    else display.print(" (Tehlike)");
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
    case 3: // Hata
      tone(BUZZER_PIN, 400, 200);
      break;
  }
}

void sesLED() {
  if(!uyariVar || !ayar.ses) return;
  
  unsigned long simdi = millis();
  int bipAraligi;
  
  if(uyariSeviye == 1) {
    bipAraligi = 1000; // Dikkat seviyesi: 1 saniyede bir
  } else {
    bipAraligi = 300; // Tehlike seviyesi: 300ms'de bir
  }
  
  if(simdi - sonBip > bipAraligi) {
    tone(BUZZER_PIN, uyariSeviye == 1 ? 600 : 800, 100);
    sonBip = simdi;
  }
}

// =========== SISTEM FONKSIYONLARI ===========
void sistemTest() {
  // Tam sistem test fonksiyonu
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.print("SISTEM TESTI");
  display.display();
  
  // Servo test
  if(ayar.servoAktif) {
    display.setCursor(10, 20);
    display.print("Servo Test...");
    display.display();
    
    for(int i = 0; i <= 180; i += 30) {
      taramaServo.write(i);
      delay(300);
    }
    taramaServo.write(90);
    delay(300);
  }
  
  // LED test
  if(ayar.ledAktif) {
    display.setCursor(10, 30);
    display.print("LED Test...");
    display.display();
    
    for(int i = 0; i < 255; i += 5) {
      analogWrite(LED_PIN, i);
      delay(20);
    }
    for(int i = 255; i > 0; i -= 5) {
      analogWrite(LED_PIN, i);
      delay(20);
    }
    analogWrite(LED_PIN, ledParlaklik);
  }
  
  // Ses test
  if(ayar.ses) {
    display.setCursor(10, 40);
    display.print("Ses Test...");
    display.display();
    
    tone(BUZZER_PIN, 523, 200); // Do
    delay(250);
    tone(BUZZER_PIN, 659, 200); // Mi
    delay(250);
    tone(BUZZER_PIN, 784, 200); // Sol
    delay(250);
    tone(BUZZER_PIN, 1046, 300); // Do
    delay(350);
    noTone(BUZZER_PIN);
  }
  
  // Sensör test
  display.setCursor(10, 50);
  display.print("Sensor Test...");
  display.display();
  
  sensorOku();
  
  delay(1000);
  
  // Test sonucu
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("TEST SONUCU:");
  display.setCursor(10, 20);
  display.print("Mesafe: ");
  display.print(mesafe);
  display.print("cm");
  display.setCursor(10, 30);
  display.print("Gaz: ");
  display.print(gaz);
  display.print("ppm");
  display.setCursor(10, 40);
  display.print("Sicaklik: ");
  display.print(sicaklik);
  display.print("C");
  display.setCursor(10, 50);
  display.print("TEST TAMAMLANDI!");
  display.display();
  
  delay(2000);
}

// =========== ACIL DURUM ===========
void acilDurum() {
  // Tüm sistemleri durdur
  noTone(BUZZER_PIN);
  taramaServo.detach();
  digitalWrite(LED_PIN, LOW);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 10);
  display.print("ACIL DURUM!");
  display.setCursor(10, 25);
  display.print("Sistem durduruldu");
  display.setCursor(10, 40);
  display.print("Yardim gerekli!");
  display.display();
  
  while(true) {
    // Kırmızı LED yanıp sönsün
    digitalWrite(SYS_LED, HIGH);
    delay(500);
    digitalWrite(SYS_LED, LOW);
    delay(500);
    
    // D18'e basılırsa sistemi yeniden başlat
    if(digitalRead(GERI_BUTON) == LOW) {
      delay(500);
      if(digitalRead(GERI_BUTON) == LOW) {
        ESP.restart();
      }
    }
  }
}

// =========== KAYDET/YUKLE ===========
void ayarlariKaydet() {
  // EEPROM'a ayarları kaydet (basitleştirilmiş)
  Serial.println("Ayarlar kaydediliyor...");
  
  // Burada EEPROM kullanılabilir
  // EEPROM.begin(512);
  // EEPROM.put(0, ayar);
  // EEPROM.commit();
  // EEPROM.end();
}

void ayarlariYukle() {
  // EEPROM'dan ayarları yükle (basitleştirilmiş)
  Serial.println("Ayarlar yukleniyor...");
  
  // Burada EEPROM kullanılabilir
  // EEPROM.begin(512);
  // EEPROM.get(0, ayar);
  // EEPROM.end();
  
  // Ayarları değişkenlere aktar
  taramaHizi = ayar.servoSpeed;
  ledMod = ayar.ledMode;
  ledParlaklik = ayar.ledBright;
}

// =========== EKRAN ANIMASYONLARI ===========
void yukleniyorEkrani(String mesaj) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print(mesaj);
  
  // Animasyonlu noktalar
  static int noktaSayisi = 0;
  static unsigned long sonAnim = 0;
  
  if(millis() - sonAnim > 500) {
    noktaSayisi = (noktaSayisi + 1) % 4;
    sonAnim = millis();
  }
  
  display.setCursor(10 + mesaj.length() * 6, 25);
  for(int i = 0; i < noktaSayisi; i++) {
    display.print(".");
  }
  
  display.display();
}

// =========== GELİŞMİŞ ÖZELLİKLER ===========
void otoKalibrasyon() {
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("OTO KALIBRASYON");
  display.setCursor(10, 25);
  display.print("Baslatiliyor...");
  display.display();
  
  // Gaz sensörü kalibrasyonu
  delay(1000);
  int toplam = 0;
  for(int i = 0; i < 10; i++) {
    toplam += analogRead(GAS_PIN);
    delay(100);
  }
  int ortalama = toplam / 10;
  ayar.gazEsik = map(ortalama, 0, 4095, 0, 2000) + 300;
  
  // Mesafe sensörü testi
  display.setCursor(10, 40);
  display.print("Mesafe testi...");
  display.display();
  
  sensorOku();
  if(mesafe > 100) {
    ayar.mesafeDikkat = 80;
    ayar.mesafeTehlike = 30;
  } else if(mesafe > 50) {
    ayar.mesafeDikkat = 60;
    ayar.mesafeTehlike = 25;
  } else {
    ayar.mesafeDikkat = 40;
    ayar.mesafeTehlike = 15;
  }
  
  display.clearDisplay();
  display.setCursor(10, 25);
  display.print("Kalibrasyon");
  display.setCursor(10, 40);
  display.print("Tamamlandi!");
  display.display();
  
  delay(1500);
}

// =========== YENİ ÖZELLİKLER İÇİN ===========
void mesafeGrafik() {
  // Mesafe grafiği çizimi
  static float mesafeKaydi[32] = {0};
  
  // Kaydırmalı dizi güncelle
  for(int i = 31; i > 0; i--) {
    mesafeKaydi[i] = mesafeKaydi[i-1];
  }
  mesafeKaydi[0] = mesafe;
  
  // Grafik çiz
  int x = 0;
  for(int i = 0; i < 32; i++) {
    int y = map(constrain(mesafeKaydi[i], 0, 100), 0, 100, 63, 20);
    display.drawPixel(x * 4, y, SSD1306_WHITE);
    if(i > 0) {
      int prevY = map(constrain(mesafeKaydi[i-1], 0, 100), 0, 100, 63, 20);
      display.drawLine((x-1)*4, prevY, x*4, y, SSD1306_WHITE);
    }
    x++;
  }
}
