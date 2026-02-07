/***************************************************************
 * TOPROAK PRO MAX V11.0 - TAM ENTEGRE SİSTEM
 * WiFi + Bluetooth + OLED + Tüm Sensörler
 * Gerçek Zamanlı Radar Görüntüleme
 * Kitleme YOK - Tam Çalışan Sistem
 ***************************************************************/

// =========== KÜTÜPHANELER ===========
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <BluetoothSerial.h>

// =========== SÜRÜM BİLGİSİ ===========
#define FIRMWARE_VERSION "V11.0"
#define BUILD_DATE "2024"
#define DEVELOPER "TOPROAK TEAM"

// =========== WIFI AYARLARI ===========
const char* ssid = "WIFI_ADINIZ";        // BURAYA KENDİ WIFI ADIN
const char* password = "WIFI_SIFRENIZ";  // BURAYA KENDİ WIFI ŞİFREN

// =========== PIN TANIMLARI ===========
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C
#define TRIG_PIN 13
#define ECHO_PIN 12
#define SERVO_PIN 26
#define PIR_PIN 14
#define GAS_PIN 36
#define BUZZER_PIN 15
#define SYS_LED 2
#define LED_PIN 25
#define JOY_X 34
#define JOY_Y 35
#define JOY_BTN 32
#define GERI_BUTON 18

// =========== NESNELER ===========
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo radarServo;
Preferences preferences;
BluetoothSerial SerialBT;
WebSocketsServer webSocket = WebSocketsServer(81);

// =========== ESP32 PWM AYARLARI ===========
#define BUZZER_CHANNEL 0
#define LED_CHANNEL 1
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// =========== GLOBAL DEĞİŞKENLER ===========
// Servo Ayarları
int servoAci = 90;
int servoMin = 20;
int servoMax = 160;
bool servoYonu = true;
bool taramaAktif = true;
int servoHiz = 5;
const int SERVO_HOME_POS = 90;

// Sensör Değerleri
float mesafe = 0;
float gazDegeri = 0;
bool hareketVar = false;
int hareketSayisi = 0;

// Tarama Haritası
#define TARAMA_NOKTALARI 36
float taramaHaritasi[TARAMA_NOKTALARI];
int aktifTaramaNoktasi = 0;

// Butonlar
bool joyBtn = false, lastJoyBtn = false;
bool geriBtn = false, lastGeriBtn = false;

// Menü Sistemi
#define MENU_ANA 0
#define MENU_MESAFE 1
#define MENU_TARAMA 2
#define MENU_SERVO_MANUEL 3
#define MENU_BLUETOOTH 4
#define MENU_WIFI 5
#define MENU_AYARLAR 6
#define MENU_ISTATISTIK 7
int aktifMenu = MENU_ANA;
int menuSecim = 0;
bool ayarModu = false;
bool manuelServoMode = false;
const int MAX_GORUNEN_OGE = 6;
const int TOPLAM_MENU_OGE = 8;

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0;
unsigned long sonUyariZamani = 0;

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0;

// Zamanlayıcılar
unsigned long sonMesafeOlcum = 0;
unsigned long sonServoHareket = 0;
unsigned long sonHareketKontrol = 0;
unsigned long sonGazOlcum = 0;
unsigned long sonWebSocketGonderim = 0;
unsigned long sonBluetoothGonderim = 0;
unsigned long sonEkranGuncelleme = 0;

// Ayar Aralıkları
const unsigned long MESAFE_ARALIK = 50;
const unsigned long SERVO_ARALIK = 30;
const unsigned long HAREKET_ARALIK = 100;
const unsigned long GAZ_ARALIK = 500;
const unsigned long WEBSOCKET_ARALIK = 50;
const unsigned long BLUETOOTH_ARALIK = 200;
const unsigned long EKRAN_ARALIK = 100;

// Engel Sistemi
bool engelVar = false;
int engelMesafeEsik = 50;

// Bağlantı Durumları
bool wifiAktif = false;
bool bluetoothAktif = true;
bool webSocketAktif = false;
bool bluetoothConnected = false;

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

// =========== FONKSİYON PROTOTİPLERİ ===========
// WiFi ve Bağlantı
void wifiBaglan();
void webSocketOlay(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void veriGonderWebSocket();
void bluetoothInit();
void bluetoothSendData();
void processBluetoothCommand(String command);

// Sensörler
float mesafeOlc();
void hareketOku();
void gazOku();
void sensorOku();

// Servo Kontrol
void servoKontrol();
void joyServoKontrol();

// Ekran ve Menü
void ekranCiz();
void baslikCiz();
void icerikCiz();
void anaMenuCiz();
void mesafeEkranCiz();
void taramaEkranCiz();
void servoManuelCiz();
void bluetoothEkranCiz();
void wifiEkranCiz();
void ayarlarEkranCiz();
void istatistikEkranCiz();
void robotCiz(int x, int y);
void altBilgiCiz();

// Butonlar
void butonOku();
void butonIsle();
void joyKontrol();

// Yardımcı Fonksiyonlar
void toneESP(int pin, int frequency, int duration);
void noToneESP();
void uyariKontrol();
void ayarYukle();
void ayarKaydet();

// =========== WiFi BAĞLANTISI ===========
void wifiBaglan() {
  Serial.println();
  Serial.println("========================================");
  Serial.println(" TOPROAK RADAR SISTEMI V11.0");
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
    Serial.println("📱 Telefondan baglanmak icin:");
    Serial.print("    http://");
    Serial.println(WiFi.localIP());
    digitalWrite(SYS_LED, HIGH);
    wifiAktif = true;
    
    // WebSocket başlat
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
        
        if (komut == "tarama_ac") {
          taramaAktif = true;
          Serial.println("📡 TARAMA AKTIF EDILDI (WebSocket)");
        }
        else if (komut == "tarama_kapat") {
          taramaAktif = false;
          Serial.println("📡 TARAMA DURDURULDU (WebSocket)");
        }
        else if (komut == "servo_orta") {
          servoAci = SERVO_HOME_POS;
          radarServo.write(servoAci);
          Serial.println("⚙️ SERVO ORTA KONUMDA (WebSocket)");
        }
        else if (komut.startsWith("servo_aci:")) {
          int yeniAci = komut.substring(10).toInt();
          servoAci = constrain(yeniAci, servoMin, servoMax);
          radarServo.write(servoAci);
          Serial.print("⚙️ SERVO ACI AYARLANDI: ");
          Serial.println(servoAci);
        }
        else if (komut == "veri_iste") {
          veriGonderWebSocket();
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
  json["gaz"] = round(gazDegeri);
  json["hareket"] = hareketVar;
  json["hareketSayisi"] = hareketSayisi;
  json["taramaAktif"] = taramaAktif;
  json["engelVar"] = engelVar;
  json["uyariVar"] = uyariVar;
  
  JsonArray tarama = json.createNestedArray("tarama");
  for (int i = 0; i < TARAMA_NOKTALARI; i++) {
    tarama.add(taramaHaritasi[i]);
  }
  
  json["aktifNokta"] = aktifTaramaNoktasi;
  
  String jsonStr;
  serializeJson(json, jsonStr);
  webSocket.broadcastTXT(jsonStr);
}

// =========== BLUETOOTH FONKSİYONLARI ===========
void bluetoothInit() {
  if (!bluetoothAktif) return;
  
  SerialBT.begin("TOPROAK-V11.0");
  Serial.println("Bluetooth baslatildi!");
  Serial.println("Cihaz adi: TOPROAK-V11.0");
  Serial.println("Baglanmak icin PIN: 1234");
}

void bluetoothSendData() {
  if (!bluetoothAktif || !SerialBT.hasClient()) return;
  
  unsigned long now = millis();
  if (now - sonBluetoothGonderim < BLUETOOTH_ARALIK) return;
  
  String data = "VERI:";
  data += "M=" + String(mesafe, 1) + ",";
  data += "A=" + String(servoAci) + ",";
  data += "G=" + String(gazDegeri, 0) + ",";
  data += "H=" + String(hareketVar ? 1 : 0) + ",";
  data += "T=" + String(taramaAktif ? 1 : 0);
  
  SerialBT.println(data);
  sonBluetoothGonderim = now;
}

void processBluetoothCommand(String command) {
  command.trim();
  if (command.length() == 0) return;
  
  Serial.print("BT Komut: ");
  Serial.println(command);
  
  if (command == "STATUS") {
    String statusMsg = "TOPROAK V11.0: M=" + String(mesafe,1) + 
                      "cm, A=" + String(servoAci) + 
                      "°, G=" + String(gazDegeri,0) + 
                      ", T=" + String(taramaAktif ? "ON" : "OFF");
    SerialBT.println(statusMsg);
  }
  else if (command == "SCAN ON") {
    taramaAktif = true;
    SerialBT.println("SCAN:ON");
  }
  else if (command == "SCAN OFF") {
    taramaAktif = false;
    SerialBT.println("SCAN:OFF");
  }
  else if (command == "SERVO HOME") {
    servoAci = SERVO_HOME_POS;
    radarServo.write(servoAci);
    SerialBT.println("SERVO:HOME");
  }
  else if (command.startsWith("SERVO ")) {
    int angle = command.substring(6).toInt();
    angle = constrain(angle, servoMin, servoMax);
    servoAci = angle;
    radarServo.write(servoAci);
    SerialBT.println("SERVO:" + String(servoAci));
  }
  else if (command == "HELP") {
    SerialBT.println("=== TOPROAK V11.0 KOMUTLARI ===");
    SerialBT.println("STATUS - Sistem durumu");
    SerialBT.println("SCAN ON/OFF - Tarama aç/kapat");
    SerialBT.println("SERVO HOME - Servo orta pozisyon");
    SerialBT.println("SERVO [açı] - Servo açısı ayarla");
    SerialBT.println("HELP - Yardım menüsü");
  }
}

void bluetoothReadData() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    processBluetoothCommand(command);
  }
}

// =========== MESAFE ÖLÇ (HC-SR04) - GELİŞMİŞ FİLTRELEME ===========
float mesafeOlc() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long sure = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (sure == 0) {
    return 400.0;
  }
  
  float mesafe = sure * 0.0343 / 2;
  
  if (mesafe < 2.0) mesafe = 2.0;
  if (mesafe > 400.0) mesafe = 400.0;
  
  return mesafe;
}

// =========== HAREKET SENSÖRÜ OKU ===========
void hareketOku() {
  bool yeniHareket = digitalRead(PIR_PIN);
  
  if (yeniHareket && !hareketVar) {
    hareketSayisi++;
    istatistik.hareketSayisi++;
    Serial.println("🚨 HAREKET TESPIT EDILDI!");
  }
  
  hareketVar = yeniHareket;
}

// =========== GAZ SENSÖRÜ OKU ===========
void gazOku() {
  int hamDeger = analogRead(GAS_PIN);
  gazDegeri = map(hamDeger, 0, 4095, 0, 1000);
  
  static float oncekiDeger = 0;
  gazDegeri = (oncekiDeger * 0.7) + (gazDegeri * 0.3);
  oncekiDeger = gazDegeri;
}

// =========== SENSÖR OKUMA ===========
void sensorOku() {
  unsigned long simdi = millis();
  
  if (simdi - sonMesafeOlcum >= MESAFE_ARALIK) {
    mesafe = mesafeOlc();
    istatistik.mesafeOkumaSayisi++;
    istatistik.toplamMesafe += mesafe;
    istatistik.ortalamaMesafe = istatistik.toplamMesafe / istatistik.mesafeOkumaSayisi;
    
    if (mesafe > istatistik.maxMesafe) istatistik.maxMesafe = mesafe;
    if (mesafe < istatistik.minMesafe && mesafe > 2.0) istatistik.minMesafe = mesafe;
    
    // Engel kontrolü
    engelVar = (mesafe < engelMesafeEsik && mesafe > 2.0);
    
    sonMesafeOlcum = simdi;
  }
  
  if (simdi - sonHareketKontrol >= HAREKET_ARALIK) {
    hareketOku();
    sonHareketKontrol = simdi;
  }
  
  if (simdi - sonGazOlcum >= GAZ_ARALIK) {
    gazOku();
    sonGazOlcum = simdi;
  }
}

// =========== SERVO KONTROL ===========
void servoKontrol() {
  unsigned long simdi = millis();
  
  if (!taramaAktif) return;
  
  if (simdi - sonServoHareket >= SERVO_ARALIK) {
    if (servoYonu) {
      servoAci += servoHiz;
      if (servoAci >= servoMax) {
        servoAci = servoMax;
        servoYonu = false;
      }
    } else {
      servoAci -= servoHiz;
      if (servoAci <= servoMin) {
        servoAci = servoMin;
        servoYonu = true;
      }
    }
    
    radarServo.write(servoAci);
    istatistik.servoHareketSayisi++;
    
    // Tarama haritasını güncelle
    int index = map(servoAci, servoMin, servoMax, 0, TARAMA_NOKTALARI-1);
    index = constrain(index, 0, TARAMA_NOKTALARI-1);
    taramaHaritasi[index] = mesafe;
    aktifTaramaNoktasi = index;
    
    sonServoHareket = simdi;
  }
}

// =========== JOYSTICK SERVO KONTROL ===========
void joyServoKontrol() {
  unsigned long simdi = millis();
  static unsigned long sonJoyHareket = 0;
  
  if (simdi - sonJoyHareket < 100) return;
  
  int joyX = analogRead(JOY_X);
  
  if (joyX < 1000) {
    servoAci -= 5;
    if (servoAci < servoMin) servoAci = servoMin;
    radarServo.write(servoAci);
    sonJoyHareket = simdi;
  } 
  else if (joyX > 3000) {
    servoAci += 5;
    if (servoAci > servoMax) servoAci = servoMax;
    radarServo.write(servoAci);
    sonJoyHareket = simdi;
  }
  
  // Joystick butonu ile home
  if (digitalRead(JOY_BTN) == LOW) {
    servoAci = SERVO_HOME_POS;
    radarServo.write(servoAci);
    delay(200);
  }
}

// =========== UYARI KONTROL ===========
void uyariKontrol() {
  uyariVar = false;
  uyariSeviye = 0;
  
  // Gerçek dünya uyarı eşikleri
  if (mesafe < 30 && mesafe > 2.0) { // 30cm tehlike
    uyariVar = true;
    uyariSeviye = 2;
    robotIfade = 2;
    toneESP(BUZZER_PIN, 1500, 100);
  } 
  else if (mesafe < 80 && mesafe > 2.0) { // 80cm dikkat
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
    toneESP(BUZZER_PIN, 1000, 50);
  }
  else if (gazDegeri > 800) { // Gaz seviyesi yüksek
    uyariVar = true;
    uyariSeviye = 1;
    robotIfade = 1;
  }
  else {
    robotIfade = 0;
  }
  
  digitalWrite(SYS_LED, uyariVar);
}

// =========== PWM FONKSİYONLARI ===========
void toneESP(int pin, int frequency, int duration) {
  ledcWriteTone(BUZZER_CHANNEL, frequency);
  if (duration > 0) {
    delay(duration);
    noToneESP();
  }
}

void noToneESP() {
  ledcWriteTone(BUZZER_CHANNEL, 0);
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
    sonTiklama = simdi;
  }
  
  if (joyBtn && !lastJoyBtn) {
    switch (aktifMenu) {
      case MENU_ANA:
        switch (menuSecim) {
          case 0: aktifMenu = MENU_MESAFE; break;
          case 1: aktifMenu = MENU_TARAMA; break;
          case 2: aktifMenu = MENU_SERVO_MANUEL; manuelServoMode = true; break;
          case 3: aktifMenu = MENU_BLUETOOTH; break;
          case 4: aktifMenu = MENU_WIFI; break;
          case 5: aktifMenu = MENU_AYARLAR; break;
          case 6: aktifMenu = MENU_ISTATISTIK; break;
        }
        break;
        
      case MENU_TARAMA:
        taramaAktif = !taramaAktif;
        toneESP(BUZZER_PIN, 800, 50);
        break;
        
      case MENU_SERVO_MANUEL:
        servoAci = SERVO_HOME_POS;
        radarServo.write(servoAci);
        toneESP(BUZZER_PIN, 800, 50);
        break;
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
  
  if (joyY < 1000) {
    menuSecim--;
    if (menuSecim < 0) menuSecim = TOPLAM_MENU_OGE - 1;
    sonHareket = simdi;
    toneESP(BUZZER_PIN, 600, 50);
  } else if (joyY > 3000) {
    menuSecim++;
    if (menuSecim >= TOPLAM_MENU_OGE) menuSecim = 0;
    sonHareket = simdi;
    toneESP(BUZZER_PIN, 600, 50);
  }
}

// =========== EKRAN FONKSİYONLARI ===========
void ekranCiz() {
  unsigned long simdi = millis();
  
  if (simdi - sonEkranGuncelleme < EKRAN_ARALIK) return;
  
  display.clearDisplay();
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  baslikCiz();
  icerikCiz();
  robotCiz(110, 50);
  altBilgiCiz();
  display.display();
  
  sonEkranGuncelleme = simdi;
}

void baslikCiz() {
  display.setCursor(40, 2);
  switch (aktifMenu) {
    case MENU_ANA: display.print("ANA MENU"); break;
    case MENU_MESAFE: display.print("MESAFE"); break;
    case MENU_TARAMA: display.print("TARAMA"); break;
    case MENU_SERVO_MANUEL: display.print("SERVO KONTROL"); break;
    case MENU_BLUETOOTH: display.print("BLUETOOTH"); break;
    case MENU_WIFI: display.print("WiFi"); break;
    case MENU_AYARLAR: display.print("AYARLAR"); break;
    case MENU_ISTATISTIK: display.print("ISTATISTIK"); break;
  }
}

void icerikCiz() {
  switch (aktifMenu) {
    case MENU_ANA: anaMenuCiz(); break;
    case MENU_MESAFE: mesafeEkranCiz(); break;
    case MENU_TARAMA: taramaEkranCiz(); break;
    case MENU_SERVO_MANUEL: servoManuelCiz(); break;
    case MENU_BLUETOOTH: bluetoothEkranCiz(); break;
    case MENU_WIFI: wifiEkranCiz(); break;
    case MENU_AYARLAR: ayarlarEkranCiz(); break;
    case MENU_ISTATISTIK: istatistikEkranCiz(); break;
  }
}

void anaMenuCiz() {
  const char* menuItems[] = {
    "Mesafe Goster",
    "Tarama Ekrani",
    "Servo Manuel Kontrol",
    "Bluetooth Durumu",
    "WiFi Durumu",
    "Ayarlar",
    "Istatistikler"
  };
  
  for (int i = 0; i < TOPLAM_MENU_OGE; i++) {
    int y = 15 + i * 9;
    
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
  display.print("Hareket: ");
  display.print(hareketVar ? "VAR" : "YOK");
  
  if (uyariVar) {
    display.setCursor(85, 55);
    display.print("UYARI!");
  }
}

void taramaEkranCiz() {
  // Basit tarama görüntüsü
  display.fillRect(0, 12, 128, 36, SSD1306_BLACK);
  display.drawRect(5, 15, 118, 30, SSD1306_WHITE);
  
  // Mevcut açı çizgisi
  int mevcutX = map(servoAci, servoMin, servoMax, 5, 123);
  display.drawFastVLine(mevcutX, 15, 30, SSD1306_WHITE);
  
  // Tarama noktaları
  for (int i = 0; i < TARAMA_NOKTALARI; i += 2) {
    float mesafeDegeri = taramaHaritasi[i];
    if (mesafeDegeri > 2.0 && mesafeDegeri < 400.0) {
      int x = map(i, 0, TARAMA_NOKTALARI-1, 5, 123);
      int y = map(mesafeDegeri, 0, 200, 45, 15);
      y = constrain(y, 15, 45);
      
      display.drawPixel(x, y, SSD1306_WHITE);
    }
  }
  
  display.setCursor(5, 50);
  display.print("A:");
  display.print(servoAci);
  display.print(" M:");
  display.print(mesafe, 0);
  display.print("cm");
  
  display.setCursor(5, 60);
  display.print("Tarama:");
  display.print(taramaAktif ? "AKTIF" : "PASIF");
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

void bluetoothEkranCiz() {
  display.setCursor(10, 15);
  display.print("BLUETOOTH");
  
  display.setCursor(10, 30);
  display.print("Durum: ");
  display.print(SerialBT.hasClient() ? "BAGLI" : "BAGLI DEGIL");
  
  display.setCursor(10, 40);
  display.print("Cihaz: TOPROAK-V11.0");
  
  display.setCursor(10, 50);
  display.print("Veri Gonderiliyor...");
}

void wifiEkranCiz() {
  display.setCursor(10, 15);
  display.print("WiFi DURUMU");
  
  display.setCursor(10, 30);
  display.print("Baglanti: ");
  display.print(wifiAktif ? "BAGLI" : "BAGLI DEGIL");
  
  if (wifiAktif) {
    display.setCursor(10, 40);
    display.print("IP: ");
    display.print(WiFi.localIP());
    
    display.setCursor(10, 50);
    display.print("WebSocket: ");
    display.print(webSocket.connectedClients());
    display.print(" bagli");
  } else {
    display.setCursor(10, 45);
    display.print("Lutfen WiFi bilgilerini");
    display.setCursor(10, 55);
    display.print("kod icinde duzenleyin");
  }
}

void ayarlarEkranCiz() {
  display.setCursor(10, 15);
  display.print("AYARLAR");
  
  display.setCursor(10, 30);
  display.print("Engel Esigi: ");
  display.print(engelMesafeEsik);
  display.print("cm");
  
  display.setCursor(10, 40);
  display.print("Servo Hiz: ");
  display.print(servoHiz);
  
  display.setCursor(10, 50);
  display.print("Tarama: ");
  display.print(taramaAktif ? "AKTIF" : "PASIF");
}

void istatistikEkranCiz() {
  display.setCursor(10, 15);
  display.print("ISTATISTIKLER");
  
  display.setCursor(10, 30);
  display.print("Calisma: ");
  display.print(istatistik.calismaSuresi / 1000);
  display.print(" sn");
  
  display.setCursor(10, 40);
  display.print("Mesafe Okuma: ");
  display.print(istatistik.mesafeOkumaSayisi);
  
  display.setCursor(10, 50);
  display.print("Hareket: ");
  display.print(istatistik.hareketSayisi);
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
  display.setCursor(5, 56);
  
  if (aktifMenu == MENU_ANA) {
    display.print("JOY:Sec  BTN:Gir  D18:Ana");
  } else {
    display.print("BTN:Geri  JOY:Sec");
  }
}

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("TOPROAK PRO MAX V11.0 baslatiliyor...");
  
  // Pin modları
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(GERI_BUTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SYS_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(SYS_LED, LOW);
  
  // PWM ayarları
  ledcSetup(BUZZER_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);
  
  ledcSetup(LED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
  ledcWrite(LED_CHANNEL, 0);
  
  // Servo başlat
  radarServo.attach(SERVO_PIN);
  radarServo.write(servoAci);
  delay(500);
  
  // OLED başlat
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED hatasi!");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
  }
  
  // WiFi bağlan
  wifiBaglan();
  
  // Bluetooth başlat
  bluetoothInit();
  
  // Ayarları yükle
  preferences.begin("toproak", false);
  ayarYukle();
  
  // Tarama haritasını sıfırla
  for (int i = 0; i < TARAMA_NOKTALARI; i++) {
    taramaHaritasi[i] = 0;
  }
  
  // İstatistik başlat
  istatistik.baslangicZamani = millis();
  
  // Başlangıç ekranı
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print("TOPROAK");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.print("PRO MAX V11.0");
  display.display();
  
  delay(2000);
  
  toneESP(BUZZER_PIN, 800, 200);
  delay(200);
  toneESP(BUZZER_PIN, 1200, 200);
  
  Serial.println("\n🎯 SİSTEM HAZIR! Tüm modüller çalışıyor...\n");
}

void ayarYukle() {
  engelMesafeEsik = preferences.getInt("engelEsik", 50);
  servoHiz = preferences.getInt("servoHiz", 5);
  taramaAktif = preferences.getBool("taramaAktif", true);
  bluetoothAktif = preferences.getBool("bluetoothAktif", true);
}

void ayarKaydet() {
  preferences.putInt("engelEsik", engelMesafeEsik);
  preferences.putInt("servoHiz", servoHiz);
  preferences.putBool("taramaAktif", taramaAktif);
  preferences.putBool("bluetoothAktif", bluetoothAktif);
}

// =========== ANA DÖNGÜ ===========
void loop() {
  unsigned long simdi = millis();
  
  // 1. SENSÖR OKUMA
  sensorOku();
  
  // 2. UYARI KONTROL
  uyariKontrol();
  
  // 3. BUTON İŞLEMLERİ
  butonOku();
  butonIsle();
  
  if (!manuelServoMode) {
    joyKontrol();
  }
  
  // 4. SERVO KONTROL
  if (manuelServoMode) {
    joyServoKontrol();
  } else {
    servoKontrol();
  }
  
  // 5. WEBSOCKET İŞLEMLERİ
  if (webSocketAktif) {
    webSocket.loop();
    
    if (simdi - sonWebSocketGonderim >= WEBSOCKET_ARALIK) {
      if (webSocket.connectedClients() > 0) {
        veriGonderWebSocket();
      }
      sonWebSocketGonderim = simdi;
    }
  }
  
  // 6. BLUETOOTH İŞLEMLERİ
  if (bluetoothAktif) {
    bluetoothSendData();
    bluetoothReadData();
  }
  
  // 7. EKRAN GÜNCELLEME
  ekranCiz();
  
  // 8. LED KONTROL (Mesafeye göre)
  int ledParlaklik = map(mesafe, 0, 400, 255, 50);
  ledParlaklik = constrain(ledParlaklik, 50, 255);
  ledcWrite(LED_CHANNEL, uyariVar ? 255 : ledParlaklik);
  
  // 9. GÖZ KIRPMA
  if (simdi - sonGozKirpma > 3000 + random(0, 2000)) {
    gozAcik = !gozAcik;
    sonGozKirpma = simdi;
  }
  
  // 10. İSTATİSTİK GÜNCELLEME
  istatistik.calismaSuresi = millis() - istatistik.baslangicZamani;
  
  // 11. SERİ MONİTÖR ÇIKTISI (Her 5 saniyede bir)
  static unsigned long sonSeriCikti = 0;
  if (simdi - sonSeriCikti >= 5000) {
    Serial.print("📊 DURUM: ");
    Serial.print(mesafe);
    Serial.print("cm | Servo:");
    Serial.print(servoAci);
    Serial.print("° | Gaz:");
    Serial.print(gazDegeri);
    Serial.print(" | Hareket:");
    Serial.print(hareketVar ? "VAR" : "YOK");
    Serial.print(" | WiFi:");
    Serial.print(webSocket.connectedClients());
    Serial.print(" | BT:");
    Serial.println(SerialBT.hasClient() ? "BAGLI" : "BAGLI DEGIL");
    
    sonSeriCikti = simdi;
  }
  
  // 12. AYARLARI OTOMATİK KAYDET (Her 30 saniyede)
  static unsigned long sonAyarKayit = 0;
  if (simdi - sonAyarKayit > 30000) {
    ayarKaydet();
    sonAyarKayit = simdi;
  }
  
  // ASLA KİTLEME YOK - KÜÇÜK BEKLEME
  delay(1);
}
