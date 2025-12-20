/****************************************************
 * TOPRAK - Akıllı Araba Asistanı
 * PROFESYONEL SON SÜRÜM
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

// =========== NESNELER ===========
Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(DHT_PIN, DHT_TYPE);

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
#define MENU_AYAR_MESAFE 4
#define MENU_AYAR_GAZ 5
#define MENU_AYAR_SICAKLIK 6
#define MENU_AYAR_SES 7

int aktifMenu = MENU_ANA;
int menuSecim = 0;
bool ayarModu = false;
int ayarSecim = 0;

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0; // 0: yok, 1: dikkat, 2: tehlike

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0; // 0: normal, 1: dikkat, 2: tehlike

// Ayarlar
struct {
  int mesafeDikkat = 50;   // cm
  int mesafeTehlike = 20;  // cm
  int gazEsik = 800;       // ppm
  int sicaklikEsik = 35;   // °C
  bool ses = true;
} ayar;

// Zamanlayıcılar
unsigned long sonOkuma = 0;
unsigned long sonBip = 0;

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
  
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
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
  Serial.println("TOPRAK - Sistem hazir!");
}

void baslangic() {
  display.clearDisplay();
  display.setCursor(40, 20);
  display.print("TOPRAK");
  display.setCursor(25, 40);
  display.print("Araba Asistani");
  display.display();
  
  if(ayar.ses) {
    tone(BUZZER_PIN, 523, 200);
    delay(300);
    tone(BUZZER_PIN, 784, 300);
  }
  
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
  
  // 4. Ekran güncelle
  ekranCiz();
  
  // 5. Ses ve LED
  sesLED();
  
  // 6. Animasyon
  if(simdi - sonGozKirpma > 3000 + random(0, 2000)) {
    gozAcik = !gozAcik;
    sonGozKirpma = simdi;
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
    Serial.print("C");
    sonSerial = millis();
  }
}

void uyariKontrol() {
  uyariVar = false;
  robotIfade = 0;
  
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
  }
  
  digitalWrite(SYS_LED, uyariVar);
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
    ayarSecim = 0;
    bipSesi(2);
    sonTiklama = simdi;
  }
  
  // JOYSTICK BUTONU
  if(joyBtn && !lastJoyBtn) {
    switch(aktifMenu) {
      case MENU_ANA:
        // Ana menüden seçime git
        switch(menuSecim) {
          case 0: aktifMenu = MENU_MESAFE; break;
          case 1: aktifMenu = MENU_GAZ; break;
          case 2: aktifMenu = MENU_SICAKLIK; break;
          case 3: aktifMenu = MENU_AYAR_MESAFE; break;
          case 4: aktifMenu = MENU_AYAR_GAZ; break;
          case 5: aktifMenu = MENU_AYAR_SICAKLIK; break;
          case 6: aktifMenu = MENU_AYAR_SES; break;
        }
        break;
        
      case MENU_AYAR_MESAFE:
      case MENU_AYAR_GAZ:
      case MENU_AYAR_SICAKLIK:
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
  
  // YUKARI/AŞAĞI - Menü gezinme
  if(joyY < 1000) { // YUKARI
    if(aktifMenu == MENU_ANA) {
      menuSecim--;
      if(menuSecim < 0) menuSecim = 6;
    } else if(ayarModu) {
      ayarSecim = 0;
    }
    bipSesi(1);
    sonHareket = simdi;
  }
  else if(joyY > 3000) { // AŞAĞI
    if(aktifMenu == MENU_ANA) {
      menuSecim++;
      if(menuSecim > 6) menuSecim = 0;
    } else if(ayarModu) {
      ayarSecim = 1;
    }
    bipSesi(1);
    sonHareket = simdi;
  }
  
  // SAĞ/SOL - Değer değiştirme (ayar modunda)
  if(ayarModu) {
    if(joyX > 3000) { // SAĞ - Artır
      ayarDegistir(1);
      bipSesi(1);
      sonHareket = simdi;
    }
    else if(joyX < 1000) { // SOL - Azalt
      ayarDegistir(-1);
      bipSesi(1);
      sonHareket = simdi;
    }
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
  display.setCursor(45, 2);
  switch(aktifMenu) {
    case MENU_ANA: display.print("ANA MENU"); break;
    case MENU_MESAFE: display.print("MESAFE"); break;
    case MENU_GAZ: display.print("GAZ"); break;
    case MENU_SICAKLIK: display.print("SICAKLIK"); break;
    case MENU_AYAR_MESAFE: display.print("AYAR MESAFE"); break;
    case MENU_AYAR_GAZ: display.print("AYAR GAZ"); break;
    case MENU_AYAR_SICAKLIK: display.print("AYAR SICAKLIK"); break;
    case MENU_AYAR_SES: display.print("AYAR SES"); break;
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
  }
}

void anaMenuCiz() {
  const char* menuItems[] = {
    "Mesafe Goster",
    "Gaz Goster",
    "Sicaklik Goster",
    "Mesafe Ayari",
    "Gaz Ayari",
    "Sicaklik Ayari",
    "Ses Ayari"
  };
  
  for(int i = 0; i < 7; i++) {
    int y = 15 + i * 7;
    
    if(i == menuSecim) {
      display.fillRect(5, y-1, 118, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(7, y);
      display.print(">");
      display.setCursor(12, y);
      display.print(menuItems[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(12, y);
      display.print(menuItems[i]);
    }
  }
}

void mesafeEkranCiz() {
  // Büyük rakam
  display.setTextSize(2);
  display.setCursor(25, 20);
  if(mesafe >= 100) {
    display.print(">100");
  } else {
    display.print(mesafe, 0);
  }
  display.print("cm");
  display.setTextSize(1);
  
  // Çubuk grafik
  int barGenislik = map(constrain(mesafe, 0, 100), 0, 100, 0, 100);
  
  display.drawRect(10, 45, 100, 10, SSD1306_WHITE);
  
  if(mesafe < ayar.mesafeTehlike) {
    display.fillRect(10, 45, barGenislik, 10, SSD1306_WHITE);
  } else if(mesafe < ayar.mesafeDikkat) {
    for(int i = 0; i < barGenislik; i += 3) {
      display.drawPixel(10 + i, 50, SSD1306_WHITE);
    }
  } else {
    display.fillRect(10, 45, barGenislik, 10, SSD1306_WHITE);
  }
  
  // Ayarlar
  display.setCursor(10, 58);
  display.print("D:");
  display.print(ayar.mesafeDikkat);
  display.print(" T:");
  display.print(ayar.mesafeTehlike);
}

void gazEkranCiz() {
  // Büyük rakam
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print(gaz, 0);
  display.print("ppm");
  display.setTextSize(1);
  
  // Hava kalitesi
  display.setCursor(10, 45);
  display.print("Hava Kalitesi: ");
  
  if(gaz < 500) {
    display.print("IYI");
  } else if(gaz < 1000) {
    display.print("ORTA");
  } else {
    display.print("KOTU");
  }
  
  // Uyarı eşiği
  display.setCursor(10, 55);
  display.print("Uyari Esigi: ");
  display.print(ayar.gazEsik);
  display.print("ppm");
}

void sicaklikEkranCiz() {
  // Büyük rakam
  display.setTextSize(2);
  display.setCursor(30, 20);
  display.print(sicaklik, 1);
  display.print("C");
  display.setTextSize(1);
  
  display.setCursor(35, 45);
  display.print("Nem: ");
  display.print(nem, 0);
  display.print("%");
  
  display.setCursor(10, 55);
  display.print("Uyari Esigi: ");
  display.print(ayar.sicaklikEsik);
  display.print("C");
}

void ayarMesafeCiz() {
  if(ayarModu) {
    // Ayar modu
    display.setCursor(10, 20);
    if(ayarSecim == 0) {
      display.print("Dikkat Seviyesi:");
      display.setTextSize(2);
      display.setCursor(30, 35);
      display.print(ayar.mesafeDikkat);
      display.print("cm");
      display.setTextSize(1);
    } else {
      display.print("Tehlike Seviyesi:");
      display.setTextSize(2);
      display.setCursor(30, 35);
      display.print(ayar.mesafeTehlike);
      display.print("cm");
      display.setTextSize(1);
    }
    
    display.setCursor(10, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    // Normal görünüm
    display.setCursor(10, 20);
    display.print("Dikkat: ");
    display.print(ayar.mesafeDikkat);
    display.print("cm");
    
    display.setCursor(10, 30);
    display.print("Tehlike: ");
    display.print(ayar.mesafeTehlike);
    display.print("cm");
    
    display.setCursor(10, 45);
    display.print("Yukari/Asagi: Sec");
    display.setCursor(10, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarGazCiz() {
  if(ayarModu) {
    // Ayar modu
    display.setCursor(10, 20);
    display.print("Gaz Uyari Esigi:");
    display.setTextSize(2);
    display.setCursor(30, 35);
    display.print(ayar.gazEsik);
    display.print("ppm");
    display.setTextSize(1);
    
    display.setCursor(10, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    // Normal görünüm
    display.setCursor(10, 25);
    display.print("Mevcut Deger: ");
    display.print(gaz, 0);
    display.print("ppm");
    
    display.setCursor(10, 40);
    display.print("Uyari Esigi: ");
    display.print(ayar.gazEsik);
    display.print("ppm");
    
    display.setCursor(10, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarSicaklikCiz() {
  if(ayarModu) {
    // Ayar modu
    display.setCursor(10, 20);
    display.print("Sicaklik Esigi:");
    display.setTextSize(2);
    display.setCursor(30, 35);
    display.print(ayar.sicaklikEsik);
    display.print("C");
    display.setTextSize(1);
    
    display.setCursor(10, 55);
    display.print("Sag/Sol: Degistir");
  } else {
    // Normal görünüm
    display.setCursor(10, 25);
    display.print("Mevcut Deger: ");
    display.print(sicaklik, 1);
    display.print("C");
    
    display.setCursor(10, 40);
    display.print("Uyari Esigi: ");
    display.print(ayar.sicaklikEsik);
    display.print("C");
    
    display.setCursor(10, 55);
    display.print("Buton: Ayar Modu");
  }
}

void ayarSesCiz() {
  display.setCursor(10, 25);
  display.print("Ses Durumu:");
  
  display.setTextSize(2);
  display.setCursor(40, 40);
  display.print(ayar.ses ? "ACIK" : "KAPALI");
  display.setTextSize(1);
  
  display.setCursor(10, 58);
  display.print("Buton: Degistir");
}

void robotCiz(int x, int y) {
  // Baş
  display.drawCircle(x, y, 8, SSD1306_WHITE);
  
  // Gözler
  if(gozAcik) {
    display.fillCircle(x-3, y-2, 2, SSD1306_WHITE);
    display.fillCircle(x+3, y-2, 2, SSD1306_WHITE);
    
    // Göz bebekleri
    int gozBebekY = y-2;
    if(robotIfade == 0) gozBebekY -= 1;
    if(robotIfade == 1) gozBebekY += 1;
    
    display.fillCircle(x-3, gozBebekY, 1, SSD1306_BLACK);
    display.fillCircle(x+3, gozBebekY, 1, SSD1306_BLACK);
  } else {
    display.drawLine(x-5, y-2, x-1, y-2, SSD1306_WHITE);
    display.drawLine(x+1, y-2, x+5, y-2, SSD1306_WHITE);
  }
  
  // Ağız
  switch(robotIfade) {
    case 0: // Normal - Gülümseme
      for(int i=0; i<7; i++) {
        display.drawPixel(x-3+i, y+2-abs(i-3)/2, SSD1306_WHITE);
      }
      break;
    case 1: // Dikkat - Düz
      display.drawLine(x-3, y+2, x+3, y+2, SSD1306_WHITE);
      break;
    case 2: // Tehlike - Aşağı
      for(int i=0; i<7; i++) {
        display.drawPixel(x-3+i, y+2+abs(i-3)/3, SSD1306_WHITE);
      }
      break;
  }
}

void altBilgiCiz() {
  display.drawFastHLine(0, 63, 128, SSD1306_WHITE);
  
  // Geri butonu bilgisi
  display.setCursor(5, 55);
  display.print("ANA:D18");
  
  // Uyarı göstergesi
  if(uyariVar) {
    display.setCursor(110, 55);
    if(uyariSeviye == 2) {
      display.print("!!!");
    } else {
      display.print("!");
    }
  }
  
  // Menü pozisyonu
  display.setCursor(85, 55);
  if(aktifMenu == MENU_ANA) {
    display.print("S:");
    display.print(menuSecim+1);
    display.print("/7");
  } else {
    display.print("MENU");
  }
}

// =========== SES FONKSİYONLARI ===========
void sesLED() {
  if(!ayar.ses || !uyariVar) {
    noTone(BUZZER_PIN);
    return;
  }
  
  unsigned long simdi = millis();
  
  if(uyariSeviye == 2) {
    // Tehlike - hızlı bip
    if(simdi - sonBip > 200) {
      tone(BUZZER_PIN, 1500, 100);
      sonBip = simdi;
    }
  } else if(uyariSeviye == 1) {
    // Dikkat - normal bip
    if(simdi - sonBip > 500) {
      tone(BUZZER_PIN, 1000, 100);
      sonBip = simdi;
    }
  }
}

void bipSesi(int tip) {
  if(!ayar.ses) return;
  
  switch(tip) {
    case 1: // Kısa bip
      tone(BUZZER_PIN, 1000, 50);
      break;
    case 2: // Çift bip (geri butonu)
      tone(BUZZER_PIN, 800, 100);
      delay(120);
      tone(BUZZER_PIN, 1200, 100);
      break;
  }
} //Kodumu Geliştirebilirsiniz.
