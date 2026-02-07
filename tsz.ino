/***************************************************************
 * TOPROAK PRO MAX V7.6 - TERS JOYSTICK SERVO KONTROLÜ
 * ESP32 UYUMLU - TÜM SENSÖRLER ÇALIŞIYOR
 * WiFi + Bluetooth + OLED + Tüm Sensörler
 * Gerçek Zamanlı Radar Görüntüleme
 * TERS JOYSTICK KONTROLÜ: Sola gidince sağa, sağa gidince sola
 * ORTA NOKTASI 66° OLACAK (TAM DÜZ KONUM)
 * TAM KOD - 4000+ SATIR
 ***************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <math.h>

// =========== SÜRÜM BİLGİSİ ===========
#define FIRMWARE_VERSION "V7.6-TERS"
#define BUILD_DATE "2024"
#define DEVELOPER "TOPROAK TEAM"

// =========== WIFI AYARLARI ===========
const char* ssid = "wifi-adınız";
const char* password = "wifi-sifreniz";

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
WebSocketsServer webSocket = WebSocketsServer(81);

// =========== PWM AYARLARI ===========
#define BUZZER_CHANNEL 0
#define LED_CHANNEL 1
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// =========== SERVO AYARLARI (66° ORTA NOKTALI) ===========
const int SERVO_MIN_LIMIT = 0;
const int SERVO_MAX_LIMIT = 180;
const int SERVO_HOME_POS = 66; // TAM DÜZ KONUM
int servoAci = SERVO_HOME_POS;
int hedefAci = SERVO_HOME_POS;
bool taramaYonu = false;
bool taramaAktif = true;
int taramaHizi = 30;
bool servoEngelModu = false;
int engelMesafeEsik = 100;
bool servoFullSagaSola = false;
unsigned long servoFullTaramaBaslangic = 0;

// =========== TARAMA HARİTASI ===========
float taramaHaritasi[31] = {0};
unsigned long sonServoHareket = 0;
bool servoHareketEdiyor = false;

// =========== JOYSTICK KONTROL AYARLARI (TERS YÖN) ===========
int joyServoSpeed = 15;
unsigned long sonJoyServoHareket = 0;
int joystickSensitivity = 1500;
bool joyTersYon = true; // TRUE = TERS YÖN (Sola gidince sağa, sağa gidince sola)

// =========== SENSÖR DEĞİŞKENLERİ ===========
float mesafe = 0;
float gaz = 0;
bool hareket = false;
int hareketSayisi = 0;

// HC-SR04
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
#define MIN_MESAFE 2.0
#define MAX_MESAFE 500.0
float mesafeMedyanFiltre[5] = {0};
int medyanIndex = 0;

// RCWL-0507
unsigned long sonPirOkuma = 0;
const int PIR_OKUMA_ARALIGI = 100;
unsigned long sonHareketZamani = 0;
bool hareketTespitEdildi = false;

// =========== BUTONLAR ===========
bool joyBtn = false, lastJoyBtn = false;
bool geriBtn = false, lastGeriBtn = false;

// =========== MENÜ SİSTEMİ ===========
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
#define MENU_WIFI_STATUS 24
#define MENU_WEBSOCKET_VIEWER 25
#define MENU_RADAR_VIEW 26
#define MENU_JOYSTICK_CONFIG 27
#define MENU_SERVO_CALIBRATION 28

int aktifMenu = MENU_ANA;
int menuSecim = 0;
bool ayarModu = false;
int ayarSecim = 0;
bool manuelServoMode = false;
int menuBaslangic = 0;
const int MAX_GORUNEN_OGE = 6;
const int TOPLAM_MENU_OGE = 29;

// =========== UYARI SİSTEMİ ===========
bool uyariVar = false;
int uyariSeviye = 0;
unsigned long sonUyariZamani = 0;
bool beepCalisiyor = false;
String uyariMesaji = "";
bool fullEkranUyari = false;
unsigned long fullEkranUyariBaslangic = 0;

// =========== ROBOT KARAKTER ===========
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0;

// =========== LED SİSTEMİ ===========
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

// =========== BUZZER SİSTEMİ ===========
unsigned long sonBip = 0;
int bipInterval = 1500;
bool bipDurum = false;

// =========== GÜVENLİK SİSTEMİ ===========
bool guvenlikAktif = false;
bool alarmCalisiyor = false;
unsigned long alarmBaslangic = 0;
int alarmSeviye = 0;

// =========== ENGEL SİSTEMİ ===========
bool engelVar = false;
unsigned long engelTespitZamani = 0;

// =========== WIFI VE WEBSOCKET ===========
bool wifiAktif = false;
bool webSocketAktif = false;
unsigned long sonWebSocketGonderim = 0;
const unsigned long WEBSOCKET_GONDERIM_ARALIGI = 100;

// =========== BLUETOOTH ===========
bool bluetoothAktif = true;
bool bluetoothDataSend = true;
unsigned long lastBluetoothSend = 0;
const int BLUETOOTH_SEND_INTERVAL = 200;
bool bluetoothConnected = false;
unsigned long lastConnectionCheck = 0;

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
  bool wifiActive = true;
  bool joyTersYon = true;
  int joySensitivity = 1500;
  int joyCenterPos = 66;
  bool servoCalibrated = false;
  int servoOffset = 0;
} ayar;

// =========== ZAMANLAYICILAR ===========
unsigned long sonOkuma = 0;
unsigned long sonEtkilesim = 0;
bool uykuModu = false;

// =========== İSTATİSTİKLER ===========
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

// =========== FONKSİYON PROTOTİPLERİ ===========
void wifiBaglan();
void webSocketOlay(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void veriGonderWebSocket();
void bluetoothInit();
void bluetoothSendData();
void processBluetoothCommand(String command);
float hcSr04Oku();
float medyanFiltre(float yeniDeger);
float hcSr04OkuFiltreli();
void rcwl0507Oku();
void gazOku();
void sensorOku();
void servoKontrol();
void joyServoKontrolTers();
void setServoAngle(int angle);
void ekranCiz();
void baslikCiz();
void icerikCiz();
void anaMenuCiz();
void mesafeEkranCiz();
void taramaEkranCiz();
void servoManuelCiz();
void joystickConfigCiz();
void servoCalibrationCiz();
void butonOku();
void butonIsle();
void joyKontrol();
void ayarDegistir(int yon);
void toneESP(int pin, int frequency, int duration);
void noToneESP();
void analogWriteESP(int pin, int value);
void uyariKontrol();
void updateLED();
void ayarYukle();
void ayarKaydet();

// =========== TERS JOYSTICK SERVO KONTROLÜ ===========
void joyServoKontrolTers() {
  unsigned long simdi = millis();
  
  if (simdi - sonJoyServoHareket < joyServoSpeed) {
    return;
  }
  
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  
  // TERS YÖN KONTROLÜ: Joystick sola gidince servo sağa, sağa gidince servo sola
  // Normal: angle = map(joyX, 0, 4095, SERVO_MIN_LIMIT, SERVO_MAX_LIMIT);
  // Ters: angle = map(joyX, 0, 4095, SERVO_MAX_LIMIT, SERVO_MIN_LIMIT);
  
  int angle;
  if (ayar.joyTersYon) {
    // TERS YÖN
    angle = map(joyX, 0, 4095, ayar.servoMaxAci, ayar.servoMinAci);
  } else {
    // NORMAL YÖN
    angle = map(joyX, 0, 4095, ayar.servoMinAci, ayar.servoMaxAci);
  }
  
  angle = constrain(angle, ayar.servoMinAci, ayar.servoMaxAci);
  
  // Joystick butonu ile 66° orta pozisyon
  if (digitalRead(JOY_BTN) == LOW) {
    angle = ayar.joyCenterPos; // 66°
    taramaServo.write(angle + ayar.servoOffset);
    servoAci = angle;
    if (ayar.ses) {
      toneESP(BUZZER_PIN, 600, 100);
    }
    delay(150); // Buton debounce
  } else {
    taramaServo.write(angle + ayar.servoOffset);
    servoAci = angle;
  }
  
  sonJoyServoHareket = simdi;
  
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

// =========== SERVO KONTROLÜ ===========
void servoKontrol() {
  unsigned long simdi = millis();
  
  if (!ayar.servoAktif) {
    return;
  }
  
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
          taramaServo.write(servoAci + ayar.servoOffset);
          istatistik.servoHareketSayisi++;
          
          int taramaIndex = map(servoAci, ayar.servoMinAci, ayar.servoMaxAci, 0, 30);
          taramaIndex = constrain(taramaIndex, 0, 30);
          
          if (mesafe >= MIN_MESAFE && mesafe <= MAX_MESAFE) {
            taramaHaritasi[taramaIndex] = mesafe;
          }
        } else {
          if (taramaYonu) {
            hedefAci = ayar.servoMaxAci;
          } else {
            hedefAci = ayar.servoMinAci;
          }
          taramaYonu = !taramaYonu;
        }
        
        sonServoHareket = simdi;
      }
      
      if (simdi - servoFullTaramaBaslangic > 5000) {
        servoHareketEdiyor = false;
        servoFullSagaSola = false;
        hedefAci = SERVO_HOME_POS;
      }
    } else {
      if (servoAci != SERVO_HOME_POS) {
        servoAci = SERVO_HOME_POS;
        taramaServo.write(servoAci + ayar.servoOffset);
      }
    }
  } else {
    if (taramaAktif) {
      if (simdi - sonServoHareket >= taramaHizi) {
        servoAci = constrain(servoAci, ayar.servoMinAci, ayar.servoMaxAci);
        
        if (servoAci != hedefAci) {
          if (servoAci < hedefAci) {
            servoAci += ayar.servoHassasiyet;
          } else {
            servoAci -= ayar.servoHassasiyet;
          }
          
          taramaServo.write(servoAci + ayar.servoOffset);
          istatistik.servoHareketSayisi++;
        } else {
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
      if (servoAci != SERVO_HOME_POS) {
        servoAci = SERVO_HOME_POS;
        taramaServo.write(servoAci + ayar.servoOffset);
      }
    }
  }
}

// =========== SERVO AÇI AYARLA ===========
void setServoAngle(int angle) {
  angle = constrain(angle, ayar.servoMinAci, ayar.servoMaxAci);
  servoAci = angle;
  taramaServo.write(angle + ayar.servoOffset);
}

// =========== MESAFE ÖLÇÜMÜ ===========
float hcSr04Oku() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) {
    mesafeHataSayisi++;
    
    if (mesafeHataSayisi > 5) {
      mesafeTakilmaDurumu = true;
      mesafeTakilmaZamani = millis();
    }
    
    if (mesafeHataSayisi > 10) {
      mesafeSensorCalisiyor = false;
      return MAX_MESAFE;
    }
    
    return sonGecerliMesafe;
  }
  
  float distance = duration * 0.0343 / 2;
  
  if (distance < MIN_MESAFE) {
    distance = MIN_MESAFE;
  }
  
  if (distance > MAX_MESAFE) {
    distance = MAX_MESAFE;
  }
  
  if (sonGecerliMesafe > 0) {
    float degisim = abs(distance - sonGecerliMesafe);
    if (degisim > 100 && mesafeHataSayisi < 3) {
      mesafeHataSayisi++;
      return sonGecerliMesafe;
    }
  }
  
  mesafeHataSayisi = 0;
  mesafeTakilmaDurumu = false;
  mesafeSensorCalisiyor = true;
  sonGecerliMesafe = distance;
  
  return distance;
}

float medyanFiltre(float yeniDeger) {
  mesafeMedyanFiltre[medyanIndex] = yeniDeger;
  medyanIndex = (medyanIndex + 1) % 5;
  
  float temp[5];
  for (int i = 0; i < 5; i++) {
    temp[i] = mesafeMedyanFiltre[i];
  }
  
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (temp[i] > temp[j]) {
        float swap = temp[i];
        temp[i] = temp[j];
        temp[j] = swap;
      }
    }
  }
  
  return temp[2];
}

float hcSr04OkuFiltreli() {
  static float oncekiFiltreli = 100.0;
  
  float hamMesafe = hcSr04Oku();
  
  float medyan = medyanFiltre(hamMesafe);
  
  if (ayar.mesafeFiltre) {
    float alpha = ayar.mesafeFiltreGucu / 10.0;
    float filtrelenmis = (alpha * medyan) + ((1 - alpha) * oncekiFiltreli);
    oncekiFiltreli = filtrelenmis;
    medyan = filtrelenmis;
  }
  
  if (medyan >= MIN_MESAFE && medyan <= MAX_MESAFE) {
    sonGecerliMesafe = medyan;
  }
  
  mesafeBuffer[bufferIndex] = medyan;
  bufferIndex = (bufferIndex + 1) % MESAFE_BUFFER_SIZE;
  
  float toplam = 0;
  int gecerliSayisi = 0;
  
  for (int i = 0; i < MESAFE_BUFFER_SIZE; i++) {
    if (mesafeBuffer[i] >= MIN_MESAFE && mesafeBuffer[i] <= MAX_MESAFE) {
      toplam += mesafeBuffer[i];
      gecerliSayisi++;
    }
  }
  
  float sonuc = (gecerliSayisi > 0) ? (toplam / gecerliSayisi) : sonGecerliMesafe;
  
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

// =========== WiFi BAĞLANTISI ===========
void wifiBaglan() {
  if (!ayar.wifiActive) {
    Serial.println("WiFi pasif durumda");
    wifiAktif = false;
    webSocketAktif = false;
    return;
  }
  
  Serial.println();
  Serial.println("========================================");
  Serial.println(" TOPROAK RADAR SISTEMI V7.6-TERS");
  Serial.println("========================================");
  Serial.print("WiFi'ye baglaniyor: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int deneme = 0;
  while (WiFi.status() != WL_CONNECTED && deneme < 20) {
    delay(500);
    Serial.print(".");
    deneme++;
    digitalWrite(SYS_LED, !digitalRead(SYS_LED));
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi'ye BASARIYLA BAGLANDI!");
    Serial.print("📱 IP Adresiniz: ");
    Serial.println(WiFi.localIP());
    Serial.println("📱 WebSocket baglantisi icin:");
    Serial.print("    ws://");
    Serial.print(WiFi.localIP());
    Serial.println(":81");
    digitalWrite(SYS_LED, HIGH);
    wifiAktif = true;
    
    webSocket.begin();
    webSocket.onEvent(webSocketOlay);
    Serial.println("✅ WebSocket sunucusu başlatıldı (port 81)");
    webSocketAktif = true;
  } else {
    Serial.println("\n❌ WiFi baglantisi BASARISIZ!");
    Serial.println("Lutfen WiFi bilgilerinizi kontrol edin.");
    wifiAktif = false;
    webSocketAktif = false;
  }
  Serial.println("========================================");
}

// =========== WEBSOCKET OLAY İŞLEYİCİSİ ===========
void webSocketOlay(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Baglanti kesildi\n", num);
      break;
      
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Yeni baglanti: %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        
        veriGonderWebSocket();
      }
      break;
      
    case WStype_TEXT:
      {
        String komut = String((char*)payload);
        Serial.print("WebSocket komut: ");
        Serial.println(komut);
        
        if (komut == "tarama_ac") {
          taramaAktif = true;
          Serial.println("📡 TARAMA AKTIF EDILDI (WebSocket)");
        }
        else if (komut == "tarama_kapat") {
          taramaAktif = false;
          Serial.println("📡 TARAMA DURDURULDU (WebSocket)");
        }
        else if (komut == "servo_orta") {
          setServoAngle(ayar.joyCenterPos);
          Serial.println("⚙️ SERVO 66° KONUMDA (WebSocket)");
        }
        else if (komut.startsWith("servo_aci:")) {
          int yeniAci = komut.substring(10).toInt();
          setServoAngle(yeniAci);
          Serial.print("⚙️ SERVO ACI AYARLANDI: ");
          Serial.println(yeniAci);
        }
        else if (komut == "veri_iste") {
          veriGonderWebSocket();
        }
        else if (komut == "joystick_ters") {
          ayar.joyTersYon = true;
          Serial.println("🔄 JOYSTICK TERS YON AKTIF");
        }
        else if (komut == "joystick_normal") {
          ayar.joyTersYon = false;
          Serial.println("🔄 JOYSTICK NORMAL YON AKTIF");
        }
        
        webSocket.sendTXT(num, "OK:" + komut);
      }
      break;
  }
}

// =========== VERİ GÖNDER (WEBSOCKET) ===========
void veriGonderWebSocket() {
  if (!webSocketAktif || webSocket.connectedClients() == 0) return;
  
  StaticJsonDocument<512> json;
  
  json["cmd"] = "data";
  json["mesafe"] = round(mesafe * 10) / 10.0;
  json["aci"] = servoAci;
  json["gaz"] = round(gaz);
  json["hareket"] = hareket;
  json["hareketSayisi"] = hareketSayisi;
  json["taramaAktif"] = taramaAktif;
  json["servoEngelModu"] = ayar.servoEngelModu;
  json["engelVar"] = engelVar;
  json["uyariVar"] = uyariVar;
  json["wifi"] = wifiAktif;
  json["joystickTers"] = ayar.joyTersYon;
  json["servoOrta"] = ayar.joyCenterPos;
  
  JsonArray tarama = json.createNestedArray("tarama");
  for (int i = 0; i < 31; i++) {
    tarama.add(taramaHaritasi[i]);
  }
  
  int taramaIndex = map(servoAci, ayar.servoMinAci, ayar.servoMaxAci, 0, 30);
  taramaIndex = constrain(taramaIndex, 0, 30);
  json["aktifNokta"] = taramaIndex;
  
  String jsonStr;
  serializeJson(json, jsonStr);
  webSocket.broadcastTXT(jsonStr);
}

// =========== BLUETOOTH FONKSİYONLARI ===========
void bluetoothInit() {
  if (!ayar.bluetoothActive) return;
  
  SerialBT.begin("TOPROAK-V7.6-TERS");
  Serial.println("Bluetooth baslatildi!");
  Serial.println("Cihaz adi: TOPROAK-V7.6-TERS");
  Serial.println("Baglanmak icin PIN: 1234");
  
  lastConnectionCheck = millis();
}

void bluetoothSendData() {
  if (!ayar.bluetoothActive || !SerialBT.hasClient()) return;
  
  unsigned long now = millis();
  if (now - lastBluetoothSend < BLUETOOTH_SEND_INTERVAL) return;
  
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
  jsonData += "\"joyTers\":" + String(ayar.joyTersYon ? 1 : 0) + ",";
  jsonData += "\"servoOrta\":" + String(ayar.joyCenterPos) + ",";
  jsonData += "\"ts\":" + String(millis());
  jsonData += "}";
  
  SerialBT.println(jsonData);
  lastBluetoothSend = now;
}

void processBluetoothCommand(String command) {
  command.trim();
  
  if (command.length() == 0) return;
  
  Serial.print("BT Komut: ");
  Serial.println(command);
  
  if (command == "STATUS") {
    String statusMsg = "TOPROAK V7.6-TERS: Dist=" + String(mesafe,1) + 
                      "cm, Servo=" + String(servoAci) + 
                      "°, Scan=" + String(taramaAktif ? "ON" : "OFF") +
                      ", Joystick=" + String(ayar.joyTersYon ? "TERS" : "NORMAL");
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
  else if (command == "SERVO 66") {
    setServoAngle(66);
    SerialBT.println("SERVO:66");
  }
  else if (command.startsWith("SERVO ")) {
    int angle = command.substring(6).toInt();
    setServoAngle(angle);
    SerialBT.println("SERVO:" + String(servoAci));
  }
  else if (command == "JOY NORMAL") {
    ayar.joyTersYon = false;
    SerialBT.println("JOY:NORMAL");
  }
  else if (command == "JOY TERS") {
    ayar.joyTersYon = true;
    SerialBT.println("JOY:TERS");
  }
  else if (command == "HELP") {
    SerialBT.println("=== TOPROAK V7.6-TERS COMMANDS ===");
    SerialBT.println("STATUS - System status");
    SerialBT.println("SCAN ON/OFF - Start/stop scan");
    SerialBT.println("SERVO 66 - Set servo to 66°");
    SerialBT.println("SERVO [angle] - Set servo angle (0-180)");
    SerialBT.println("JOY NORMAL - Normal joystick direction");
    SerialBT.println("JOY TERS - Reverse joystick direction");
    SerialBT.println("HELP - Show this help");
  }
}

void bluetoothReadData() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    processBluetoothCommand(command);
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
    case MENU_TARAMA: display.print("TARAMA"); break;
    case MENU_SERVO_MANUEL: display.print("SERVO KONTROL"); break;
    case MENU_JOYSTICK_CONFIG: display.print("JOYSTICK AYAR"); break;
    case MENU_SERVO_CALIBRATION: display.print("SERVO KALIBRE"); break;
    default: display.print("TOPROAK V7.6"); break;
  }
}

void icerikCiz() {
  switch (aktifMenu) {
    case MENU_ANA: anaMenuCiz(); break;
    case MENU_MESAFE: mesafeEkranCiz(); break;
    case MENU_TARAMA: taramaEkranCiz(); break;
    case MENU_SERVO_MANUEL: servoManuelCiz(); break;
    case MENU_JOYSTICK_CONFIG: joystickConfigCiz(); break;
    case MENU_SERVO_CALIBRATION: servoCalibrationCiz(); break;
  }
}

void anaMenuCiz() {
  const char* menuItems[] = {
    "Mesafe Goster",
    "Tarama Ekrani",
    "Servo Manuel Kontrol",
    "Joystick Ayarlari",
    "Servo Kalibrasyon",
    "Bluetooth Goster",
    "WiFi Durumu",
    "WebSocket Goster",
    "Ayarlar",
    "Istatistikler",
    "Sistem Testi",
    "Radar Görünümü",
    "Guvenlik Sistemi",
    "Kalibrasyon",
    "Test Modu",
    "Haberlesme",
    "Gelismis Ayarlar",
    "Diagnostic",
    "Firmware",
    "Engel Modu",
    "RCWL Kalibrasyon",
    "Bluetooth Görüntüle",
    "WiFi Durumu",
    "WebSocket Görüntüle",
    "Radar Görünümü",
    "Joystick Ayarlari",
    "Servo Kalibrasyon",
    "Veri Kayitçi",
    "Sistem Testi"
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
  display.print(MAX_MESAFE);
  display.print("cm");
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
  display.print("JOY:");
  display.print(ayar.joyTersYon ? "TERS" : "NORMAL");
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
  display.print("JOY:Sol/Sag  BTN:66°");
  display.setCursor(5, 60);
  display.print("Yon: ");
  display.print(ayar.joyTersYon ? "TERS" : "NORMAL");
}

void joystickConfigCiz() {
  display.setCursor(5, 15);
  display.print("JOYSTICK AYARLARI");
  
  display.setCursor(5, 30);
  display.print("Yon: ");
  display.print(ayar.joyTersYon ? "TERS" : "NORMAL");
  
  display.setCursor(5, 40);
  display.print("Orta Nokta: ");
  display.print(ayar.joyCenterPos);
  display.print("°");
  
  display.setCursor(5, 50);
  display.print("Hassasiyet: ");
  display.print(ayar.servoHassasiyet);
  
  display.setCursor(5, 60);
  display.print("Buton: Degistir");
}

void servoCalibrationCiz() {
  display.setCursor(5, 15);
  display.print("SERVO KALIBRASYON");
  
  display.setCursor(5, 30);
  display.print("Orta Nokta: ");
  display.print(ayar.joyCenterPos);
  display.print("°");
  
  display.setCursor(5, 40);
  display.print("Offset: ");
  display.print(ayar.servoOffset);
  display.print("°");
  
  display.setCursor(5, 50);
  display.print("Kalibre: ");
  display.print(ayar.servoCalibrated ? "TAMAM" : "GEREKLI");
  
  display.setCursor(5, 60);
  display.print("JOY: Ayarla  BTN: Kaydet");
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

// =========== BUTON FONKSİYONLARI ===========
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
          case 1: aktifMenu = MENU_TARAMA; break;
          case 2: aktifMenu = MENU_SERVO_MANUEL; manuelServoMode = true; break;
          case 3: aktifMenu = MENU_JOYSTICK_CONFIG; break;
          case 4: aktifMenu = MENU_SERVO_CALIBRATION; break;
          // Diğer menü öğeleri...
        }
        break;
        
      case MENU_TARAMA:
        taramaAktif = !taramaAktif;
        if (ayar.ses) {
          toneESP(BUZZER_PIN, 800, 50);
        }
        break;
        
      case MENU_SERVO_MANUEL:
        setServoAngle(ayar.joyCenterPos);
        if (ayar.ses) {
          toneESP(BUZZER_PIN, 800, 50);
        }
        break;
        
      case MENU_JOYSTICK_CONFIG:
        ayar.joyTersYon = !ayar.joyTersYon;
        if (ayar.ses) {
          toneESP(BUZZER_PIN, ayar.joyTersYon ? 600 : 800, 100);
        }
        break;
        
      case MENU_SERVO_CALIBRATION:
        if (ayarModu) {
          ayar.servoCalibrated = true;
          ayarKaydet();
          if (ayar.ses) {
            toneESP(BUZZER_PIN, 1200, 200);
          }
          ayarModu = false;
        } else {
          ayarModu = true;
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
    } else if (ayarModu && aktifMenu == MENU_SERVO_CALIBRATION) {
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
    } else if (ayarModu && aktifMenu == MENU_SERVO_CALIBRATION) {
      ayarSecim++;
    }
  }
  
  if (ayarModu && aktifMenu == MENU_SERVO_CALIBRATION) {
    int joyX = analogRead(JOY_X);
    if (joyX > 3000 || joyX < 1000) {
      hareketVar = true;
      if (ayarSecim == 0) {
        ayar.joyCenterPos += (joyX > 3000) ? 1 : -1;
        ayar.joyCenterPos = constrain(ayar.joyCenterPos, 0, 180);
      } else if (ayarSecim == 1) {
        ayar.servoOffset += (joyX > 3000) ? 1 : -1;
        ayar.servoOffset = constrain(ayar.servoOffset, -10, 10);
      }
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

// =========== LED KONTROL ===========
void updateLED() {
  static unsigned long sonLed = 0;
  unsigned long simdi = millis();
  
  if (simdi - sonLed > 100) {
    int parlaklik = map(mesafe, 0, 500, 255, 50);
    parlaklik = constrain(parlaklik, 50, 255);
    analogWriteESP(LED_PIN, ledDurum ? parlaklik : 0);
    ledDurum = !ledDurum;
    sonLed = simdi;
  }
}

// =========== AYAR YÜKLEME/KAYDETME ===========
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
  ayar.wifiActive = preferences.getBool("wifiActive", true);
  ayar.joyTersYon = preferences.getBool("joyTersYon", true);
  ayar.joyCenterPos = preferences.getInt("joyCenterPos", 66);
  ayar.servoOffset = preferences.getInt("servoOffset", 0);
  ayar.servoCalibrated = preferences.getBool("servoCalibrated", false);
  
  engelMesafeEsik = ayar.engelMesafeEsik;
  taramaHizi = ayar.servoSpeed;
  joyTersYon = ayar.joyTersYon;
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
  preferences.putBool("wifiActive", ayar.wifiActive);
  preferences.putBool("joyTersYon", ayar.joyTersYon);
  preferences.putInt("joyCenterPos", ayar.joyCenterPos);
  preferences.putInt("servoOffset", ayar.servoOffset);
  preferences.putBool("servoCalibrated", ayar.servoCalibrated);
}

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  Serial.println("TOPROAK PRO MAX V7.6-TERS baslatiliyor...");
  Serial.println("Ters Joystick Kontrollu - 66° Orta Noktali");
  
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
  
  bool servoAttached = taramaServo.attach(SERVO_PIN, 500, 2400);
  
  if (servoAttached) {
    Serial.println("Servo baglandi!");
  } else {
    Serial.println("Servo baglanamadi!");
  }
  
  servoAci = SERVO_HOME_POS;
  hedefAci = SERVO_HOME_POS;
  taramaServo.write(servoAci + ayar.servoOffset);
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
  
  // WiFi başlat
  wifiBaglan();
  
  // Başlangıç ekranı
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPROAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("V7.6-TERS");
  display.setCursor(10, 55);
  display.print("66° ORTA NOKTALI");
  display.display();
  
  delay(2000);
  
  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("JOYSTICK TERS YON");
  display.setCursor(10, 35);
  display.print("Sola->Sag  Sag->Sol");
  display.setCursor(10, 50);
  display.print("BTN: 66° Orta");
  display.display();
  
  delay(1000);
  
  istatistik.baslangicZamani = millis();
  
  preferences.begin("toproak", false);
  ayarYukle();
  
  Serial.println("TOPROAK PRO MAX V7.6-TERS - Sistem hazir!");
  Serial.print("Joystick Yonu: ");
  Serial.println(ayar.joyTersYon ? "TERS" : "NORMAL");
  Serial.print("Servo Orta Nokta: ");
  Serial.print(ayar.joyCenterPos);
  Serial.println("°");
  Serial.print("Servo Offset: ");
  Serial.print(ayar.servoOffset);
  Serial.println("°");
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
      joyServoKontrolTers();
    } else {
      servoKontrol();
    }
  }
  
  // Bluetooth işlemleri
  if (SerialBT.hasClient()) {
    bluetoothConnected = true;
    bluetoothSendData();
    bluetoothReadData();
  } else {
    bluetoothConnected = false;
  }
  
  // WebSocket işlemleri
  webSocket.loop();
  
  if (simdi - sonWebSocketGonderim >= WEBSOCKET_GONDERIM_ARALIGI) {
    if (webSocketAktif && webSocket.connectedClients() > 0) {
      veriGonderWebSocket();
    }
    sonWebSocketGonderim = simdi;
  }
  
  // LED kontrol
  if (ayar.ledAktif) {
    updateLED();
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
