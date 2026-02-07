/****************************************************
 * TOPROAK PRO MAX V7.6 - BLUETOOTH DESTEKLİ
 * ESP32 UYUMLU - TÜM SENSÖRLER ÇALIŞIYOR
 * RCWL-0507 HAREKET SENSÖRÜ DESTEKLİ
 * TAM SERVO VE MANUEL KONTROL
 * BLUETOOTH İLE TELEFON GÖRSELLEŞTİRMESİ
 ****************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <BluetoothSerial.h>

// =========== SÜRÜM BİLGİSİ ===========
#define FIRMWARE_VERSION "V7.6"
#define BUILD_DATE "2024"
#define DEVELOPER "TOPROAK TEAM"

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
Preferences preferences;
BluetoothSerial SerialBT;

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
#define MENU_ENGEL_MODU 21
#define MENU_RCWL_KALIBRASYON 22
#define MENU_BLUETOOTH_VIEWER 23

// =========== GLOBAL DEĞİŞKENLER ===========
float mesafe = 0;
float gaz = 0;
bool hareket = false;

// HC-SR04 Gelişmiş
#define MESAFE_BUFFER_SIZE 10
float mesafeBuffer[MESAFE_BUFFER_SIZE];
int bufferIndex = 0;
unsigned long sonMesafeOkuma = 0;
const int MESAFE_OKUMA_ARALIGI = 50;
bool mesafeSensorCalisiyor = true;
int mesafeHataSayisi = 0;
float sonGecerliMesafe = 100.0;
bool mesafeTakilmaDurumu = false;
unsigned long mesafeTakilmaZamani = 0;
float mesafeFiltreli = 0;
#define MIN_MESAFE 2.0
#define MAX_MESAFE 500.0
float mesafeMedyanFiltre[5] = {0};
int medyanIndex = 0;

// RCWL-0507
unsigned long sonPirOkuma = 0;
const int PIR_OKUMA_ARALIGI = 100;
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
const int TOPLAM_MENU_OGE = 24;

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0;
unsigned long sonUyariZamani = 0;
bool beepCalisiyor = false;
String uyariMesaji = "";
bool fullEkranUyari = false;
unsigned long fullEkranUyariBaslangic = 0;

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0;

// =========== SERVO SİSTEMİ ===========
const int SERVO_MIN_LIMIT = -90;
const int SERVO_MAX_LIMIT = 90;
const int SERVO_HOME_POS = 0;
int servoAci = SERVO_HOME_POS;
int hedefAci = SERVO_HOME_POS;
bool taramaYonu = false;
bool taramaAktif = true;
int taramaHizi = 30;
bool servoEngelModu = false;
int engelMesafeEsik = 100;
bool servoFullSagaSola = false;
unsigned long servoFullTaramaBaslangic = 0;

// TARAMA HARİTASI
float taramaHaritasi[31] = {0};
unsigned long sonServoHareket = 0;
bool servoHareketEdiyor = false;

// BLUETOOTH SİSTEMİ
bool bluetoothAktif = true;
bool bluetoothDataSend = true;
unsigned long lastBluetoothSend = 0;
const int BLUETOOTH_SEND_INTERVAL = 200;
String bluetoothBuffer = "";
bool bluetoothConnected = false;
unsigned long lastConnectionCheck = 0;

// LED Sistemi
int ledParlaklik = 200;
int ledMod = 2;
bool ledDurum = true;
unsigned long sonLedDegisim = 0;
int ledBlinkSpeed = 1000;

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
int joyServoSpeed = 15;
unsigned long sonJoyServoHareket = 0;
int joystickSensitivity = 1500;

// Engel Sistemi
bool engelVar = false;
unsigned long engelTespitZamani = 0;

// =========== AYARLAR ===========
struct {
  int mesafeDikkat = 100;
  int mesafeTehlike = 50;
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
  int maxMesafe = 500;
  bool servoEngelModu = false;
  int engelMesafeEsik = 100;
  bool bluetoothActive = true;
  bool mesafeFiltre = true;
  int mesafeFiltreGucu = 5;
  int servoHassasiyet = 3;
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

// =========== BLUETOOTH FONKSİYONLARI ===========
void bluetoothInit() {
  if (!ayar.bluetoothActive) return;
  
  SerialBT.begin("TOPROAK-V7.6");
  Serial.println("Bluetooth baslatildi!");
  Serial.println("Cihaz adi: TOPROAK-V7.6");
  Serial.println("Baglanmak icin PIN: 1234 veya 0000");
  
  lastConnectionCheck = millis();
}

void bluetoothSendData() {
  if (!ayar.bluetoothActive || !bluetoothConnected) return;
  
  unsigned long now = millis();
  if (now - lastBluetoothSend < BLUETOOTH_SEND_INTERVAL) return;
  
  // JSON formatında veri gönder
  String jsonData = "{";
  jsonData += "\"ver\":\"" + String(FIRMWARE_VERSION) + "\",";
  jsonData += "\"dist\":" + String(mesafe, 1) + ",";
  jsonData += "\"gas\":" + String(gaz, 0) + ",";
  jsonData += "\"motion\":" + String(hareket ? 1 : 0) + ",";
  jsonData += "\"servo\":" + String(servoAci) + ",";
  jsonData += "\"angle\":" + String(servoAci) + ",";
  jsonData += "\"scan\":" + String(taramaAktif ? 1 : 0) + ",";
  jsonData += "\"obstacle\":" + String(engelVar ? 1 : 0) + ",";
  jsonData += "\"warning\":" + String(uyariVar ? 1 : 0) + ",";
  jsonData += "\"mode\":" + String(ayar.servoEngelModu ? 2 : (taramaAktif ? 1 : 0)) + ",";
  jsonData += "\"ts\":" + String(millis());
  jsonData += "}";
  
  SerialBT.println(jsonData);
  lastBluetoothSend = now;
}

void sendScanDataToBluetooth() {
  if (!ayar.bluetoothActive || !bluetoothConnected) return;
  
  if (!taramaAktif) return;
  
  // Mevcut açı ve mesafeyi gönder
  String scanData = "SCAN:" + String(servoAci) + "," + String(mesafe, 1);
  SerialBT.println(scanData);
}

void sendFullScanDataToBluetooth() {
  if (!ayar.bluetoothActive || !bluetoothConnected) return;
  
  // Tüm tarama haritasını gönder
  String fullScanData = "FULLSCAN:";
  for (int i = 0; i < 31; i++) {
    fullScanData += String(taramaHaritasi[i], 1);
    if (i < 30) fullScanData += ",";
  }
  SerialBT.println(fullScanData);
}

void bluetoothCheckConnection() {
  unsigned long now = millis();
  
  if (now - lastConnectionCheck > 5000) {
    bluetoothConnected = SerialBT.hasClient();
    
    if (bluetoothConnected) {
      digitalWrite(SYS_LED, HIGH);
      delay(50);
      digitalWrite(SYS_LED, LOW);
    }
    
    lastConnectionCheck = now;
  }
}

void processBluetoothCommand(String command) {
  command.trim();
  
  if (command.length() == 0) return;
  
  Serial.print("BT Komut: ");
  Serial.println(command);
  
  if (command == "STATUS") {
    String statusMsg = "TOPROAK V7.6 Status: Dist=" + String(mesafe,1) + 
                      "cm, Gas=" + String(gaz,0) + 
                      "ppm, Servo=" + String(servoAci) + 
                      "°, Scan=" + String(taramaAktif ? "ON" : "OFF");
    SerialBT.println(statusMsg);
  }
  else if (command == "SCAN ON") {
    taramaAktif = true;
    SerialBT.println("SCAN:ACTIVE");
  }
  else if (command == "SCAN OFF") {
    taramaAktif = false;
    SerialBT.println("SCAN:INACTIVE");
  }
  else if (command == "SERVO ON") {
    ayar.servoAktif = true;
    SerialBT.println("SERVO:ON");
  }
  else if (command == "SERVO OFF") {
    ayar.servoAktif = false;
    SerialBT.println("SERVO:OFF");
  }
  else if (command == "GET SCAN") {
    sendFullScanDataToBluetooth();
  }
  else if (command == "LED ON") {
    ayar.ledAktif = true;
    SerialBT.println("LED:ON");
  }
  else if (command == "LED OFF") {
    ayar.ledAktif = false;
    SerialBT.println("LED:OFF");
  }
  else if (command == "BUZZER ON") {
    ayar.ses = true;
    SerialBT.println("BUZZER:ON");
  }
  else if (command == "BUZZER OFF") {
    ayar.ses = false;
    SerialBT.println("BUZZER:OFF");
  }
  else if (command.startsWith("SERVO ")) {
    int angle = command.substring(6).toInt();
    angle = constrain(angle, ayar.servoMinAci, ayar.servoMaxAci);
    servoAci = angle;
    taramaServo.write(servoAci);
    SerialBT.println("SERVO:" + String(servoAci));
  }
  else if (command == "HELP") {
    SerialBT.println("=== TOPROAK V7.6 COMMANDS ===");
    SerialBT.println("STATUS - System status");
    SerialBT.println("SCAN ON/OFF - Start/stop scan");
    SerialBT.println("SERVO ON/OFF - Enable/disable servo");
    SerialBT.println("SERVO [angle] - Set servo angle");
    SerialBT.println("GET SCAN - Get full scan data");
    SerialBT.println("LED ON/OFF - Turn LED on/off");
    SerialBT.println("BUZZER ON/OFF - Turn buzzer on/off");
    SerialBT.println("HELP - Show this help");
  }
}

void bluetoothReadData() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    processBluetoothCommand(command);
  }
}

// =========== GELİŞMİŞ MESAFE ÖLÇÜMÜ - TAKILMA SORUNU ÇÖZÜLDÜ ===========
float hcSr04Oku() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) {
    mesafeHataSayisi++;
    
    // Takılma durumu - uzun süre aynı değer dönüyorsa
    if (mesafeHataSayisi > 5) {
      mesafeTakilmaDurumu = true;
      mesafeTakilmaZamani = millis();
    }
    
    if (mesafeHataSayisi > 10) {
      mesafeSensorCalisiyor = false;
      return MAX_MESAFE; // Takılma durumunda MAX mesafe döndür
    }
    
    return sonGecerliMesafe;
  }
  
  float distance = duration * 0.0343 / 2;
  
  // Çok yakın veya çok uzun mesafeleri filtrele
  if (distance < MIN_MESAFE) {
    distance = MIN_MESAFE;
  }
  
  if (distance > MAX_MESAFE) {
    distance = MAX_MESAFE;
  }
  
  // Anormal değişimleri filtrele
  if (sonGecerliMesafe > 0) {
    float degisim = abs(distance - sonGecerliMesafe);
    if (degisim > 100 && mesafeHataSayisi < 3) {
      // Ani büyük değişim - muhtemelen hata
      mesafeHataSayisi++;
      return sonGecerliMesafe;
    }
  }
  
  // Takılma durumunu sıfırla
  mesafeHataSayisi = 0;
  mesafeTakilmaDurumu = false;
  mesafeSensorCalisiyor = true;
  sonGecerliMesafe = distance;
  
  return distance;
}

float medyanFiltre(float yeniDeger) {
  // Medyan filtre array'ine ekle
  mesafeMedyanFiltre[medyanIndex] = yeniDeger;
  medyanIndex = (medyanIndex + 1) % 5;
  
  // Kopya oluştur ve sırala
  float temp[5];
  for (int i = 0; i < 5; i++) {
    temp[i] = mesafeMedyanFiltre[i];
  }
  
  // Bubble sort
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (temp[i] > temp[j]) {
        float swap = temp[i];
        temp[i] = temp[j];
        temp[j] = swap;
      }
    }
  }
  
  // Medyan değer (ortadaki)
  return temp[2];
}

float hcSr04OkuFiltreli() {
  static float oncekiFiltreli = 100.0;
  
  float hamMesafe = hcSr04Oku();
  
  // Medyan filtre uygula
  float medyan = medyanFiltre(hamMesafe);
  
  // Hareketli ortalama
  if (ayar.mesafeFiltre) {
    float alpha = ayar.mesafeFiltreGucu / 10.0;
    float filtrelenmis = (alpha * medyan) + ((1 - alpha) * oncekiFiltreli);
    oncekiFiltreli = filtrelenmis;
    medyan = filtrelenmis;
  }
  
  // Son geçerli mesafe güncelle
  if (medyan >= MIN_MESAFE && medyan <= MAX_MESAFE) {
    sonGecerliMesafe = medyan;
  }
  
  // Buffer'a ekle
  mesafeBuffer[bufferIndex] = medyan;
  bufferIndex = (bufferIndex + 1) % MESAFE_BUFFER_SIZE;
  
  // Buffer'dan ortalama hesapla
  float toplam = 0;
  int gecerliSayisi = 0;
  
  for (int i = 0; i < MESAFE_BUFFER_SIZE; i++) {
    if (mesafeBuffer[i] >= MIN_MESAFE && mesafeBuffer[i] <= MAX_MESAFE) {
      toplam += mesafeBuffer[i];
      gecerliSayisi++;
    }
  }
  
  float sonuc = (gecerliSayisi > 0) ? (toplam / gecerliSayisi) : sonGecerliMesafe;
  
  // İstatistik güncelle
  istatistik.mesafeOkumaSayisi++;
  istatistik.toplamMesafe += sonuc;
  istatistik.ortalamaMesafe = istatistik.toplamMesafe / istatistik.mesafeOkumaSayisi;
  
  if (sonuc > istatistik.maxMesafe) istatistik.maxMesafe = sonuc;
  if (sonuc < istatistik.minMesafe && sonuc > MIN_MESAFE) istatistik.minMesafe = sonuc;
  
  return sonuc;
}

// =========== RCWL-0507 OKUMA ===========
void rcwl0507Oku() {
  unsigned long simdi = millis();
  
  if (simdi - sonPirOkuma >= PIR_OKUMA_ARALIGI) {
    int hareketDegeri = digitalRead(PIR_PIN);
    
    if (hareketDegeri == HIGH) {
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

// =========== GAZ OKUMA ===========
void gazOku() {
  static int gazBuffer[10] = {0};
  static int gazBufferIndex = 0;
  
  int gazDeger = analogRead(GAS_PIN);
  
  gazBuffer[gazBufferIndex] = gazDeger;
  gazBufferIndex = (gazBufferIndex + 1) % 10;
  
  int toplam = 0;
  for (int i = 0; i < 10; i++) {
    toplam += gazBuffer[i];
  }
  
  gaz = map(toplam / 10, 0, 4095, 0, 2000);
}

// =========== SENSÖR OKUMA ===========
void sensorOku() {
  unsigned long simdi = millis();
  
  if (simdi - sonMesafeOkuma >= MESAFE_OKUMA_ARALIGI) {
    mesafe = hcSr04OkuFiltreli();
    
    // Engel kontrolü
    if (mesafe < engelMesafeEsik && mesafe > MIN_MESAFE) {
      if (!engelVar) {
        engelVar = true;
        engelTespitZamani = simdi;
        
        if (ayar.servoEngelModu && !servoHareketEdiyor) {
          servoHareketEdiyor = true;
          servoFullTaramaBaslangic = simdi;
          servoFullSagaSola = true;
        }
      }
    } else {
      engelVar = false;
    }
    
    sonMesafeOkuma = simdi;
  }
  
  gazOku();
  rcwl0507Oku();
  
  sonOkuma = simdi;
}

// =========== UYARI KONTROL ===========
void uyariKontrol() {
  bool oncekiUyari = uyariVar;
  
  uyariVar = false;
  uyariSeviye = 0;
  
  // SADECE 500cm (5 metre) ALTINDAKİ MESAFELER İÇİN UYARI
  if (mesafe < ayar.mesafeTehlike && mesafe <= 500 && mesafe > MIN_MESAFE) {
    uyariVar = true;
    uyariSeviye = 2;
    robotIfade = 2;
    sonUyariZamani = millis();
    
    if (!fullEkranUyari) {
      fullEkranUyari = true;
      fullEkranUyariBaslangic = millis();
    }
  } else if (mesafe < ayar.mesafeDikkat && mesafe <= 500 && mesafe > MIN_MESAFE) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
    sonUyariZamani = millis();
  } else if (gaz > ayar.gazEsik) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
    sonUyariZamani = millis();
  } else if (engelVar && ayar.servoEngelModu) {
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
    sonUyariZamani = millis();
  } else {
    robotIfade = 0;
  }
  
  // Full ekran uyarı zaman aşımı
  if (fullEkranUyari && (millis() - fullEkranUyariBaslangic > 3000)) {
    fullEkranUyari = false;
  }
  
  if ((oncekiUyari != uyariVar) && !uyariVar) {
    if (ayar.ses) {
      noToneESP();
      beepCalisiyor = false;
    }
  }
  
  digitalWrite(SYS_LED, uyariVar);
}

// =========== SERVO KONTROLÜ - TAM ÇALIŞAN ===========
void servoKontrol() {
  unsigned long simdi = millis();
  
  if (!ayar.servoAktif) {
    return;
  }
  
  // Engel modu kontrolü
  if (ayar.servoEngelModu) {
    if (engelVar && !servoHareketEdiyor) {
      servoHareketEdiyor = true;
      servoFullTaramaBaslangic = simdi;
      servoFullSagaSola = true;
      hedefAci = ayar.servoMaxAci;
      taramaYonu = false;
    }
    
    if (servoHareketEdiyor) {
      if (simdi - sonServoHareket >= taramaHizi) {
        if (servoAci != hedefAci) {
          if (servoAci < hedefAci) {
            servoAci += ayar.servoHassasiyet;
          } else {
            servoAci -= ayar.servoHassasiyet;
          }
          
          servoAci = constrain(servoAci, ayar.servoMinAci, ayar.servoMaxAci);
          taramaServo.write(servoAci);
          istatistik.servoHareketSayisi++;
          
          // Tarama haritasını güncelle
          int taramaIndex = map(servoAci, ayar.servoMinAci, ayar.servoMaxAci, 0, 30);
          taramaIndex = constrain(taramaIndex, 0, 30);
          
          if (mesafe >= MIN_MESAFE && mesafe <= MAX_MESAFE) {
            taramaHaritasi[taramaIndex] = mesafe;
          }
        } else {
          // Hedef açıya ulaşıldı, yön değiştir
          if (taramaYonu) {
            hedefAci = ayar.servoMaxAci;
          } else {
            hedefAci = ayar.servoMinAci;
          }
          taramaYonu = !taramaYonu;
        }
        
        sonServoHareket = simdi;
      }
      
      // Tarama süresi doldu mu? (5 saniye)
      if (simdi - servoFullTaramaBaslangic > 5000) {
        servoHareketEdiyor = false;
        servoFullSagaSola = false;
        hedefAci = SERVO_HOME_POS;
      }
    } else {
      // Engel yoksa, servo home pozisyonunda kalsın
      if (servoAci != SERVO_HOME_POS) {
        servoAci = SERVO_HOME_POS;
        taramaServo.write(servoAci);
      }
    }
  } else {
    // Normal tarama modu
    if (taramaAktif) {
      if (simdi - sonServoHareket >= taramaHizi) {
        servoAci = constrain(servoAci, ayar.servoMinAci, ayar.servoMaxAci);
        
        // Mevcut pozisyona git
        if (servoAci != hedefAci) {
          if (servoAci < hedefAci) {
            servoAci += ayar.servoHassasiyet;
          } else {
            servoAci -= ayar.servoHassasiyet;
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
        
        // TARAMA HARİTASI GÜNCELLEMESİ
        int taramaIndex = map(servoAci, ayar.servoMinAci, ayar.servoMaxAci, 0, 30);
        taramaIndex = constrain(taramaIndex, 0, 30);
        
        if (mesafe >= MIN_MESAFE && mesafe <= MAX_MESAFE) {
          if (taramaHaritasi[taramaIndex] == 0) {
            taramaHaritasi[taramaIndex] = mesafe;
          } else {
            taramaHaritasi[taramaIndex] = (taramaHaritasi[taramaIndex] * 0.7) + (mesafe * 0.3);
          }
        }
        
        sonServoHareket = simdi;
      }
    } else {
      // Tarama kapalıysa servoyu orta pozisyonda tut
      if (servoAci != SERVO_HOME_POS) {
        servoAci = SERVO_HOME_POS;
        taramaServo.write(servoAci);
      }
    }
  }
}

// =========== JOYSTICK İLE SERVO KONTROL - TAM ÇALIŞAN ===========
void joyServoKontrol() {
  unsigned long simdi = millis();
  
  if (simdi - sonJoyServoHareket < joyServoSpeed) {
    return;
  }
  
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  
  // Joystick kontrolü
  if (joyX < (2048 - joystickSensitivity / 2)) {
    // Sola hareket
    servoAci -= ayar.servoHassasiyet;
    if (servoAci < ayar.servoMinAci) {
      servoAci = ayar.servoMinAci;
      if (ayar.ses) {
        toneESP(BUZZER_PIN, 800, 50);
      }
    }
    taramaServo.write(servoAci);
    istatistik.servoHareketSayisi++;
    sonJoyServoHareket = simdi;
  } 
  else if (joyX > (2048 + joystickSensitivity / 2)) {
    // Sağa hareket
    servoAci += ayar.servoHassasiyet;
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
  
  // Joystick butonu ile home pozisyonu
  if (digitalRead(JOY_BTN) == LOW) {
    servoAci = SERVO_HOME_POS;
    taramaServo.write(servoAci);
    if (ayar.ses) {
      toneESP(BUZZER_PIN, 600, 100);
    }
    delay(150);
  }
  
  // Yukarı/Aşağı ile hassasiyet ayarı
  if (joyY < 1000) {
    ayar.servoHassasiyet++;
    if (ayar.servoHassasiyet > 10) ayar.servoHassasiyet = 10;
    delay(200);
  } else if (joyY > 3000) {
    ayar.servoHassasiyet--;
    if (ayar.servoHassasiyet < 1) ayar.servoHassasiyet = 1;
    delay(200);
  }
}

// =========== PWM FONKSİYONLARI ===========
void toneESP(int pin, int frequency, int duration) {
  if (!ayar.ses) return;
  
  ledcWriteTone(BUZZER_CHANNEL, frequency);
  if (duration > 0) {
    delay(duration);
    noToneESP();
  }
}

void noToneESP() {
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void analogWriteESP(int pin, int value) {
  ledcWrite(LED_CHANNEL, value);
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
    if (ayar.ses) {
      toneESP(BUZZER_PIN, 1000, 100);
    }
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
          case 20: aktifMenu = MENU_ENGEL_MODU; break;
          case 21: aktifMenu = MENU_RCWL_KALIBRASYON; break;
          case 22: aktifMenu = MENU_BLUETOOTH_VIEWER; break;
        }
        break;
        
      case MENU_TARAMA:
        taramaAktif = !taramaAktif;
        if (ayar.ses) {
          toneESP(BUZZER_PIN, 800, 50);
        }
        break;
        
      case MENU_SERVO_MANUEL:
        servoAci = SERVO_HOME_POS;
        taramaServo.write(servoAci);
        if (ayar.ses) {
          toneESP(BUZZER_PIN, 800, 50);
        }
        break;
        
      case MENU_GUVENLIK:
        guvenlikAktif = !guvenlikAktif;
        if (guvenlikAktif) {
          if (ayar.ses) {
            toneESP(BUZZER_PIN, 400, 200);
          }
        } else {
          alarmCalisiyor = false;
          noToneESP();
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
          noToneESP();
          beepCalisiyor = false;
        }
        if (ayar.ses) {
          toneESP(BUZZER_PIN, 800, 50);
        }
        break;
        
      case MENU_ENGEL_MODU:
        ayar.servoEngelModu = !ayar.servoEngelModu;
        if (ayar.ses) {
          toneESP(BUZZER_PIN, ayar.servoEngelModu ? 600 : 800, 100);
        }
        break;
    }
    
    if (ayar.ses) {
      toneESP(BUZZER_PIN, 800, 50);
    }
    sonTiklama = simdi;
  }
  
  lastJoyBtn = joyBtn;
  lastGeriBtn = geriBtn;
}

void joyKontrol() {
  static unsigned long sonHareket = 0;
  unsigned long simdi = millis();
  
  if (simdi - sonHareket < 200) return;
  
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
  
  if (ayarModu) {
    int joyX = analogRead(JOY_X);
    if (joyX > 3000 || joyX < 1000) {
      hareketVar = true;
      ayarDegistir(joyX > 3000 ? 1 : -1);
    }
  }
  
  if (hareketVar) {
    sonEtkilesim = simdi;
    uykuModu = false;
    if (ayar.ses) {
      toneESP(BUZZER_PIN, 800, 50);
    }
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
        ayar.servoMinAci = constrain(ayar.servoMinAci, 0, ayar.servoMaxAci - 10);
      } else if (ayarSecim == 2) {
        ayar.servoMaxAci += yon * 5;
        ayar.servoMaxAci = constrain(ayar.servoMaxAci, ayar.servoMinAci + 10, 150);
      } else if (ayarSecim == 3) {
        ayar.servoHassasiyet += yon;
        ayar.servoHassasiyet = constrain(ayar.servoHassasiyet, 1, 10);
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
      
    case MENU_ENGEL_MODU:
      if (ayarSecim == 1) {
        engelMesafeEsik += yon * 5;
        engelMesafeEsik = constrain(engelMesafeEsik, 20, 200);
        ayar.engelMesafeEsik = engelMesafeEsik;
      }
      break;
  }
  if (ayar.ses) {
    toneESP(BUZZER_PIN, 800, 50);
  }
}

// =========== EKRAN FONKSİYONLARI ===========
void ekranCiz() {
  if (fullEkranUyari && uyariSeviye == 2) {
    fullEkranUyariGoster();
    return;
  }
  
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
    case MENU_ENGEL_MODU: display.print("ENGEL MODU"); break;
    case MENU_RCWL_KALIBRASYON: display.print("RCWL KALIBRE"); break;
    case MENU_BLUETOOTH_VIEWER: display.print("BLUETOOTH"); break;
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
    case MENU_ENGEL_MODU: engelModuCiz(); break;
    case MENU_RCWL_KALIBRASYON: rcwlKalibrasyonCiz(); break;
    case MENU_BLUETOOTH_VIEWER: bluetoothViewerCiz(); break;
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
    "Firmware",
    "Engel Modu",
    "RCWL Kalibrasyon",
    "Bluetooth Goruntule"
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

void taramaEkranCiz() {
  display.fillRect(0, 12, 128, 36, SSD1306_BLACK);
  display.drawRect(5, 15, 118, 30, SSD1306_WHITE);
  
  for (int i = 1; i <= 3; i++) {
    int y = 15 + (30 * i / 4);
    display.drawFastHLine(5, y, 118, SSD1306_WHITE);
  }
  
  for (int i = 1; i <= 3; i++) {
    int x = 5 + (118 * i / 4);
    display.drawFastVLine(x, 15, 30, SSD1306_WHITE);
  }
  
  int mevcutX = map(servoAci, ayar.servoMinAci, ayar.servoMaxAci, 5, 123);
  display.drawFastVLine(mevcutX, 15, 30, SSD1306_WHITE);
  
  int oncekiX = -1;
  int oncekiY = -1;
  
  for (int i = 0; i < 31; i++) {
    float mesafeDegeri = taramaHaritasi[i];
    if (mesafeDegeri > 2.0 && mesafeDegeri < 200.0) {
      int x = map(i, 0, 30, 5, 123);
      int y = map(mesafeDegeri, 0, 200, 45, 15);
      
      int noktaBoyut = map(mesafeDegeri, 0, 200, 3, 1);
      noktaBoyut = constrain(noktaBoyut, 1, 3);
      
      if (noktaBoyut == 3) {
        display.fillCircle(x, y, 2, SSD1306_WHITE);
      } else if (noktaBoyut == 2) {
        display.drawCircle(x, y, 2, SSD1306_WHITE);
      } else {
        display.drawPixel(x, y, SSD1306_WHITE);
      }
      
      if (oncekiX != -1 && oncekiY != -1) {
        display.drawLine(oncekiX, oncekiY, x, y, SSD1306_WHITE);
      }
      
      oncekiX = x;
      oncekiY = y;
    }
  }
  
  for (int i = 0; i < 31; i += 2) {
    float mesafeDegeri = taramaHaritasi[i];
    if (mesafeDegeri > 2.0 && mesafeDegeri < 200.0) {
      int x = map(i, 0, 30, 5, 123);
      int barYukseklik = map(mesafeDegeri, 0, 200, 5, 1);
      barYukseklik = constrain(barYukseklik, 1, 5);
      
      display.drawFastVLine(x, 46, barYukseklik, SSD1306_WHITE);
    }
  }
  
  display.setCursor(5, 46);
  display.print("0");
  display.setCursor(60, 46);
  display.print("75");
  display.setCursor(115, 46);
  display.print("150");
  
  display.fillRect(0, 50, 128, 14, SSD1306_BLACK);
  display.setCursor(2, 52);
  display.print("A:");
  display.print(servoAci);
  display.print(" M:");
  display.print(mesafe, 0);
  display.print("cm");
  
  display.setCursor(2, 60);
  if (servoHareketEdiyor) {
    display.print("TARAMA:AKTIF");
  } else if (ayar.servoEngelModu) {
    display.print("ENGEL MOD:BEKLE");
  } else {
    display.print("TARAMA:");
    display.print(taramaAktif ? "AKTIF" : "PASIF");
  }
  
  if (uyariVar) {
    display.setCursor(85, 52);
    display.print("UYARI!");
  }
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
  display.setCursor(5, 60);
  display.print("Hassasiyet: ");
  display.print(ayar.servoHassasiyet);
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
    } else {
      display.print("Hassasiyet:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(ayar.servoHassasiyet);
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
    display.print("Hassasiyet: ");
    display.print(ayar.servoHassasiyet);
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
  display.print("TOPROAK PRO MAX V7.6");
  display.setCursor(5, 20);
  display.print("Calisma: ");
  display.print((millis() - istatistik.baslangicZamani) / 1000);
  display.print(" sn");
  display.setCursor(5, 30);
  display.print("Mesafe Okuma: ");
  display.print(istatistik.mesafeOkumaSayisi);
  display.setCursor(5, 40);
  display.print("Hareket: ");
  display.print(istatistik.hareketSayisi);
  display.setCursor(5, 50);
  display.print("Servo Hareket: ");
  display.print(istatistik.servoHareketSayisi);
  display.setCursor(5, 60);
  display.print("Bluetooth: ");
  display.print(bluetoothConnected ? "BAGLI" : "BAGLI DEGIL");
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
  display.print("Bluetooth: ");
  display.print(bluetoothConnected ? "BAGLI" : "KAPALI");
  display.setCursor(10, 55);
  display.print("Cihaz: TOPROAK-V7.6");
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
  display.print("Versiyon: V7.6");
  display.setCursor(10, 45);
  display.print("Derleme: 2024");
  display.setCursor(10, 55);
  display.print("Guncelleme: Yok");
}

void engelModuCiz() {
  display.setCursor(5, 15);
  display.print("Engel Modu: ");
  display.print(ayar.servoEngelModu ? "AKTIF" : "PASIF");
  
  display.setCursor(5, 25);
  display.print("Engel Esik: ");
  display.print(engelMesafeEsik);
  display.print(" cm");
  
  display.setCursor(5, 35);
  display.print("Mevcut Mesafe: ");
  display.print(mesafe, 0);
  display.print(" cm");
  
  display.setCursor(5, 45);
  display.print("Engel Durumu: ");
  display.print(engelVar ? "VAR" : "YOK");
  
  display.setCursor(5, 55);
  display.print("Servo: ");
  display.print(servoHareketEdiyor ? "TARAMA" : "BEKLIYOR");
}

void rcwlKalibrasyonCiz() {
  display.setCursor(5, 15);
  display.print("RCWL-0507 Kalibrasyon");
  
  display.setCursor(5, 25);
  display.print("Durum: CALISIYOR");
  
  display.setCursor(5, 35);
  display.print("Hareket: ");
  display.print(hareket ? "VAR" : "YOK");
  
  display.setCursor(5, 45);
  display.print("Toplam: ");
  display.print(hareketSayisi);
  
  display.setCursor(5, 55);
  display.print("Otomatik kalibre");
}

void bluetoothViewerCiz() {
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setCursor(10, 5);
  display.print("BLUETOOTH");
  
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print("Durum: ");
  
  if (bluetoothConnected) {
    display.print("BAGLI");
    display.setCursor(10, 35);
    display.print("Cihaz: TOPROAK-V7.6");
    display.setCursor(10, 45);
    display.print("Veri Gonderiliyor...");
    
    static int btAnim = 0;
    btAnim = (btAnim + 1) % 10;
    
    display.setCursor(10, 55);
    if (btAnim < 5) {
      display.print("[===>        ]");
    } else {
      display.print("[        ===>]");
    }
  } else {
    display.print("BAGLI DEGIL");
    display.setCursor(10, 35);
    display.print("Baglanti bekleniyor");
    display.setCursor(10, 45);
    display.print("Cihaz Adi:");
    display.setCursor(10, 55);
    display.print("TOPROAK-V7.6");
  }
}

void fullEkranUyariGoster() {
  if (!fullEkranUyari) return;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(15, 5);
  display.print("! UYARI !");
  
  display.setTextSize(1);
  display.setCursor(10, 30);
  display.print("Mesafe: ");
  display.print(mesafe, 0);
  display.print(" cm");
  
  display.setCursor(10, 40);
  if (mesafe < ayar.mesafeTehlike) {
    display.print("TEHLIKELI MESAFE!");
  } else {
    display.print("DIKKATLI OLUN!");
  }
  
  display.setCursor(10, 50);
  display.print("Servo: ");
  display.print(servoHareketEdiyor ? "TARAMA" : "BEKLIYOR");
  
  static bool cerceveDurum = false;
  if (millis() % 500 < 250) {
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    display.drawRect(1, 1, 126, 62, SSD1306_WHITE);
  }
  
  display.display();
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
  
  if (uyariVar && !fullEkranUyari) {
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

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  Serial.println("TOPROAK PRO MAX V7.6 baslatiliyor...");
  Serial.println("Bluetooth Destekli");
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
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
  
  ledcSetup(BUZZER_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);
  
  ledcSetup(LED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
  ledcWrite(LED_CHANNEL, 0);
  
  for (int i = 0; i < MESAFE_BUFFER_SIZE; i++) {
    mesafeBuffer[i] = 100.0;
  }
  
  for (int i = 0; i < 31; i++) {
    taramaHaritasi[i] = 0;
  }
  
  Serial.println("Servo baslatiliyor...");
  
  ESP32PWM::allocateTimer(0);
  taramaServo.setPeriodHertz(50);
  
  bool servoAttached = taramaServo.attach(SERVO_PIN, 500, 2500);
  
  if (servoAttached) {
    Serial.println("Servo baglandi!");
  } else {
    Serial.println("Servo baglanamadi!");
  }
  
  servoAci = SERVO_HOME_POS;
  hedefAci = SERVO_HOME_POS;
  taramaServo.write(servoAci);
  Serial.print("Servo baslangic pozisyonu: ");
  Serial.println(servoAci);
  
  delay(1000);
  
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED hatasi!");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
  }
  
  // Bluetooth başlat
  bluetoothInit();
  
  // Başlangıç ekranı
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPROAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("PRO MAX V7.6");
  display.display();
  
  delay(2000);
  
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("BLUETOOTH AKTIF");
  display.setCursor(10, 35);
  display.print("Cihaz: TOPROAK-V7.6");
  display.setCursor(10, 50);
  display.print("Baglanti bekleniyor");
  display.display();
  
  delay(1000);
  
  istatistik.baslangicZamani = millis();
  
  preferences.begin("toproak", false);
  ayarYukle();
  
  Serial.println("TOPROAK PRO MAX V7.6 - Sistem hazir!");
  Serial.print("Bluetooth: ");
  Serial.println(ayar.bluetoothActive ? "Aktif" : "Pasif");
  Serial.print("Engel Modu: ");
  Serial.println(ayar.servoEngelModu ? "Aktif" : "Pasif");
}

void ayarYukle() {
  ayar.mesafeDikkat = preferences.getInt("mesafeDikkat", 100);
  ayar.mesafeTehlike = preferences.getInt("mesafeTehlike", 50);
  ayar.gazEsik = preferences.getInt("gasEsik", 800);
  ayar.ses = preferences.getBool("ses", true);
  ayar.servoAktif = preferences.getBool("servoAktif", true);
  ayar.servoSpeed = preferences.getInt("servoSpeed", 15);
  ayar.servoMinAci = preferences.getInt("servoMinAci", SERVO_MIN_LIMIT);
  ayar.servoMaxAci = preferences.getInt("servoMaxAci", SERVO_MAX_LIMIT);
  ayar.servoEngelModu = preferences.getBool("servoEngelModu", false);
  ayar.engelMesafeEsik = preferences.getInt("engelMesafeEsik", 100);
  ayar.bluetoothActive = preferences.getBool("bluetoothActive", true);
  
  engelMesafeEsik = ayar.engelMesafeEsik;
  taramaHizi = ayar.servoSpeed;
}

void ayarKaydet() {
  preferences.putInt("mesafeDikkat", ayar.mesafeDikkat);
  preferences.putInt("mesafeTehlike", ayar.mesafeTehlike);
  preferences.putInt("gasEsik", ayar.gazEsik);
  preferences.putBool("ses", ayar.ses);
  preferences.putBool("servoAktif", ayar.servoAktif);
  preferences.putInt("servoSpeed", ayar.servoSpeed);
  preferences.putInt("servoMinAci", ayar.servoMinAci);
  preferences.putInt("servoMaxAci", ayar.servoMaxAci);
  preferences.putBool("servoEngelModu", ayar.servoEngelModu);
  preferences.putInt("engelMesafeEsik", ayar.engelMesafeEsik);
  preferences.putBool("bluetoothActive", ayar.bluetoothActive);
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
    } else {
      servoKontrol();
    }
  }
  
  // Bluetooth işlemleri
  bluetoothCheckConnection();
  if (bluetoothConnected) {
    bluetoothSendData();
    bluetoothReadData();
  }
  
  // LED kontrol
  if (ayar.ledAktif) {
    static unsigned long sonLed = 0;
    if (simdi - sonLed > 100) {
      int parlaklik = map(mesafe, 0, 500, 255, 50);
      parlaklik = constrain(parlaklik, 50, 255);
      analogWriteESP(LED_PIN, ledDurum ? parlaklik : 0);
      ledDurum = !ledDurum;
      sonLed = simdi;
    }
  } else {
    analogWriteESP(LED_PIN, 0);
  }
  
  // Buzzer kontrol
  if (ayar.ses && uyariVar && !beepCalisiyor) {
    if (simdi - sonBip > bipInterval) {
      beepCalisiyor = true;
      int frekans = (mesafe < ayar.mesafeTehlike) ? 1500 : 1200;
      int sure = (mesafe < ayar.mesafeTehlike) ? 100 : 150;
      toneESP(BUZZER_PIN, frekans, sure);
      sonBip = simdi;
      beepCalisiyor = false;
    }
  }
  
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
  
  // 10 dakikada bir ayarları kaydet
  static unsigned long sonAyarKayit = 0;
  if (simdi - sonAyarKayit > 600000) {
    ayarKaydet();
    sonAyarKayit = simdi;
  }
  
  delay(10);
}
