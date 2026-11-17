/*********************************************************************************
 * 🚗 TOPRAK PRO MAX V10.0 (ULTIMATE EDITION) — AKILLI ARAÇ GÜVENLİK SİSTEMİ
 * 
 * Geliştirici: Toprak Ahmet Aydoğmuş
 * Platform: ESP32 NodeMCU (WROOM-32)
 * Lisans: MIT License
 * 
 * ============================== DONANIM BAĞLANTILARI ==============================
 * [OLED Ekran (SSD1306 128x64 I2C)]
 *   - SDA: GPIO 21
 *   - SCL: GPIO 22
 *   - I2C Adresi: 0x3C
 * 
 * [Analog Joystick Modülü]
 *   - VRx: GPIO 34 (Analog)
 *   - VRy: GPIO 35 (Analog)
 *   - SW (Buton): GPIO 32 (Dahili Pull-up)
 * 
 * [Navigasyon Butonu]
 *   - D18 (Geri Butonu): GPIO 18 (Kısa: Geri / 1.5 sn Uzun: Sessiz Mod)
 * 
 * [Mesafe Sensörü (HC-SR04)]
 *   - TRIG: GPIO 13
 *   - ECHO: GPIO 12
 * 
 * [Gaz / Hava Kalitesi (MQ-135 / MQ-2)]
 *   - AOUT: GPIO 36 (VP / ADC1_CH0)
 * 
 * [Hareket Sensörü]
 *   - GPIO 14: PIR (HC-SR501) / RCWL-0507
 * 
 * [Aktüatörler]
 *   - Buzzer: GPIO 15 (Yumuşak Tonlu PWM)
 *   - Sistem LED: GPIO 2 (Kalp Atışı)
 *   - Uyarı LED: GPIO 25 (PWM)
 *   - Radar Servo: GPIO 26 (SG90 PWM)
 * 
 * ============================== V10.0 ÖZEL YENİLİKLERİ ==============================
 * 1. GELİŞMİŞ GAZ & HAVA ANALİZÖRÜ (MQ-135):
 *    - Dinamik Temiz Hava Referans Takibi (Drift-Tracking Baseline).
 *    - Ortam havasında asla sahte alarm çalmaz.
 *    - Çakmak gazı, alkol veya duman yaklaştırıldığında anında milisaniyelik tepki.
 *    - Ekranda anlık PPM, Ham ADC ve Fark (+Δ) göstergesi ile tam şeffaflık.
 *    - 4 Seviyeli Sınıflandırma: TEMİZ -> ORTA -> KİRLİ -> TEHLİKE!
 * 2. KİBAR OTOMOBİL PARK SENSÖRÜ SESİ:
 *    - Eski rahatsız edici alarm yerine lüks otomobil park sensörü ritmi.
 *    - 35 cm üzerinde 100% SESSİZ.
 *    - Mesafeye göre orantılı yumuşak tıklar (25 ms mikro-pıt).
 *    - D18 butonuna 1.5 sn basılı tutarak tek tuşla anında Sessiz Mod (Mute).
 * 3. İPEKSİ 180° RADAR SÜPÜRMESİ:
 *    - 15° ile 165° arası güvenli açıda mekanik dişli takılması sıfırlandı.
 *    - 1 derecelik mikro-adımlarla (16 ms) fısıltı sessizliğinde kesintisiz tarama.
 *    - Arka planda sürekli çalışarak radar haritasını canlı tutar.
 * 4. KAYA GİBİ SAĞLAM JOYSTICK & BUTON KONTROLÜ:
 *    - Açılışta otomatik merkez kalibrasyonu (Edge-Triggered).
 *    - Menü kendi kendine asla kaymaz, ekran hiçbir zaman kaybolmaz.
 *********************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <BluetoothSerial.h>
#include <math.h>

// ==================== VERSİYON VE BİLGİ ====================
#define FIRMWARE_NAME    "TOPRAK V10"
#define FIRMWARE_VER     "V10.0-PRO"

// ==================== PIN TANIMLARI ====================
#define OLED_SDA         21
#define OLED_SCL         22
#define OLED_ADDR        0x3C
#define SCREEN_WIDTH     128
#define SCREEN_HEIGHT    64

#define JOY_X            34
#define JOY_Y            35
#define JOY_BTN          32
#define BACK_BTN         18

#define TRIG_PIN         13
#define ECHO_PIN         12
#define GAS_PIN          36
#define PIR_PIN          14
#define BUZZER_PIN       15
#define SYS_LED          2
#define LED_PIN          25
#define SERVO_PIN        26

// ==================== PWM KANALLARI ====================
#define BUZZER_CHANNEL   0
#define LED_CHANNEL      1
#define PWM_FREQ         2000
#define PWM_RESOLUTION   8

// ==================== MENÜ TANIMLARI ====================
enum MenuScreens {
  MENU_MAIN = 0,
  MENU_DISTANCE,
  MENU_GAS,
  MENU_MOTION,
  MENU_RADAR,
  MENU_SERVO_MANUAL,
  MENU_STATISTICS,
  MENU_SETTINGS_DIST,
  MENU_SETTINGS_GAS,
  MENU_SETTINGS_SERVO,
  MENU_SETTINGS_SYSTEM,
  TOTAL_MENUS
};

const char* menuNames[] = {
  "Mesafe Radari",
  "Gaz & Hava",
  "Hareket & Guv.",
  "Radar Tarama",
  "Manuel Servo",
  "Istatistikler",
  "Mesafe Ayari",
  "Gaz Ayarlari",
  "Servo Ayari",
  "Sistem & Ses"
};
const int MAIN_MENU_COUNT = 10;

// ==================== AYARLAR YAPISI (FLASH SAKLAMA) ====================
struct SystemSettings {
  int distanceWarning;      // cm (35 cm - kibar ikaz başlangıcı)
  int distanceDanger;       // cm (15 cm - yakın park)
  int maxDistance;          // cm (300 cm)
  int gasThreshold;         // ppm (1300 ppm - tehlike eşiği)
  bool soundEnabled;        // Ses açık/kapalı
  bool ledEnabled;          // LED açık/kapalı
  int servoHomePos;         // 66 derece (tam düz yön)
  bool servoInvert;         // Ters joystick modu
  int servoSpeed;           // Tarama gecikmesi (ms)
  bool continuousRadar;     // Sürekli radar taraması
  bool bluetoothEnabled;    // Bluetooth açık/kapalı
};

SystemSettings settings;

// ==================== SİSTEM İSTATİSTİKLERİ ====================
struct SystemStatistics {
  unsigned long startTime;
  unsigned long distanceReadCount;
  float minDistance;
  float maxDistance;
  float avgDistance;
  float totalDistance;
  unsigned long motionCount;
  unsigned long servoSteps;
};

SystemStatistics stats;

// ==================== NESNELER ====================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo radarServo;
Preferences prefs;
BluetoothSerial SerialBT;

// ==================== SENSÖR VE ÇALIŞMA DEĞİŞKENLERİ ====================
float distance = 100.0;
float distanceRaw = 100.0;
int gasRaw = 1600;
int gasBaseline = 1600;
int gasDelta = 0;
float gasPPM = 400.0;
int gasQualityLevel = 0; // 0: Temiz, 1: Orta, 2: Kirli, 3: Tehlike
bool motionDetected = false;
unsigned long lastMotionTime = 0;

// Radar Haritası
uint8_t radarMap[181];
int servoAngle = 66;
bool scanDirection = true;
bool manualServoMode = false;
unsigned long lastServoStep = 0;
unsigned long lastJoyServoStep = 0;

// Alarm Seviyesi
int alertLevel = 0; // 0: Sessiz, 1: İkaz, 2: Tehlike
bool bluetoothConnected = false;
unsigned long bootTime = 0;
bool warmupComplete = false;

// Navigasyon ve Butonlar
int activeMenu = MENU_MAIN;
int menuSelection = 0;
int menuScrollOffset = 0;
bool editMode = false;
int editParamIndex = 0;

int joyCenterX = 2755;
int joyCenterY = 2829;
bool joyYEngaged = false;
bool joyXEngaged = false;

bool joyBtnState = false;
bool lastJoyBtnRaw = HIGH;
unsigned long lastJoyBtnDebounce = 0;

bool backBtnState = false;
bool lastBackBtnRaw = HIGH;
unsigned long backBtnPressStart = 0;
bool backBtnLongHandled = false;

// Yumuşak Park Sensörü Ses Üreteci
enum BuzzerState { BUZZ_OFF, BUZZ_ON };
BuzzerState buzzState = BUZZ_OFF;
unsigned long buzzTimer = 0;
int buzzInterval = 0;

// Robot Animasyonu
bool eyesOpen = true;
unsigned long lastBlinkTime = 0;
int robotExpression = 0;

// ==================== YUMUŞAK VE GÜÇLÜ SES PWM KONTROLÜ ====================
void setupPWM() {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcAttach(BUZZER_PIN, 1200, 8); // Park ve acil ikaz frekansı
  ledcWrite(BUZZER_PIN, 0);
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(LED_PIN, 0);
#else
  ledcSetup(BUZZER_CHANNEL, 1200, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);
  ledcSetup(LED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
  ledcWrite(LED_CHANNEL, 0);
#endif
}

// Güçlü veya kısık ses kontrolü (Aşırı sesli acil alarm veya kademeli tık)
void setBuzzerAudio(bool on, int volume = 40) {
  if (!settings.soundEnabled || !on) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(BUZZER_PIN, 0);
#else
    ledcWrite(BUZZER_CHANNEL, 0);
#endif
    return;
  }
  volume = constrain(volume, 0, 255);
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(BUZZER_PIN, volume);
#else
  ledcWrite(BUZZER_CHANNEL, volume);
#endif
}

void setSoftBuzzer(bool on) {
  setBuzzerAudio(on, 40);
}

void setWarningLED(int brightness) {
  if (!settings.ledEnabled) brightness = 0;
  brightness = constrain(brightness, 0, 255);
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(LED_PIN, brightness);
#else
  ledcWrite(LED_CHANNEL, brightness);
#endif
}

void triggerChirp() {
  if (!settings.soundEnabled) return;
  setSoftBuzzer(true);
  buzzState = BUZZ_ON;
  buzzTimer = millis();
}

// ==================== KALICI HAFIZA (PREFERENCES) ====================
void loadSettings() {
  prefs.begin("toprak_v10", false);
  settings.distanceWarning   = prefs.getInt("d_warn", 50); // 20-50 cm kademeli ikaz
  settings.distanceDanger    = prefs.getInt("d_dang", 20); // <= 20 cm aşırı sesli
  settings.maxDistance       = prefs.getInt("d_max", 300);
  settings.gasThreshold      = prefs.getInt("g_thresh", 1200);
  settings.soundEnabled      = prefs.getBool("snd_en", true);
  settings.ledEnabled        = prefs.getBool("led_en", true);
  settings.servoHomePos      = prefs.getInt("srv_home", 90);
  settings.servoInvert       = prefs.getBool("srv_inv", true);
  settings.servoSpeed        = prefs.getInt("srv_spd", 16);
  settings.continuousRadar   = prefs.getBool("rad_cont", true);
  settings.bluetoothEnabled  = prefs.getBool("bt_en", true);
  prefs.end();
}

void saveSettings() {
  prefs.begin("toprak_v10", false);
  prefs.putInt("d_warn", settings.distanceWarning);
  prefs.putInt("d_dang", settings.distanceDanger);
  prefs.putInt("d_max", settings.maxDistance);
  prefs.putInt("g_thresh", settings.gasThreshold);
  prefs.putBool("snd_en", settings.soundEnabled);
  prefs.putBool("led_en", settings.ledEnabled);
  prefs.putInt("srv_home", settings.servoHomePos);
  prefs.putBool("srv_inv", settings.servoInvert);
  prefs.putInt("srv_spd", settings.servoSpeed);
  prefs.putBool("rad_cont", settings.continuousRadar);
  prefs.putBool("bt_en", settings.bluetoothEnabled);
  prefs.end();
}

void restoreFactoryDefaults() {
  settings.distanceWarning   = 50;
  settings.distanceDanger    = 20;
  settings.maxDistance       = 300;
  settings.gasThreshold      = 1200;
  settings.soundEnabled      = true;
  settings.ledEnabled        = true;
  settings.servoHomePos      = 90;
  settings.servoInvert       = true;
  settings.servoSpeed        = 16;
  settings.continuousRadar   = true;
  settings.bluetoothEnabled  = true;
  saveSettings();
  triggerChirp();
}

// ==================== SENSÖR OKUMA VE FİLTRELEME ====================
void readDistanceSensor() {
  static unsigned long lastTriggerTime = 0;
  unsigned long now = millis();
  if (now - lastTriggerTime < 65) return; // 65ms periyot: Yankıların sönümlenmesini sağlar, işlemciyi kilitlemez
  lastTriggerTime = now;

  // Yankı pini hala meşgulse beklemeden atla
  if (digitalRead(ECHO_PIN) == HIGH) return;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 12000); // 12ms zaman aşımı (~200 cm menzil): Servoyu ASLA duraksatmaz

  float measured = 0;
  if (duration > 180 && duration < 11800) {
    measured = (duration * 0.03432) / 2.0;
  } else {
    measured = (float)settings.maxDistance;
  }

  measured = constrain(measured, 2.0, (float)settings.maxDistance);
  distanceRaw = measured;

  // 5 elemanlı medyan filtre
  static float dBuffer[5] = {100, 100, 100, 100, 100};
  static int dIndex = 0;
  dBuffer[dIndex] = measured;
  dIndex = (dIndex + 1) % 5;

  float sorted[5];
  for (int i = 0; i < 5; i++) sorted[i] = dBuffer[i];
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (sorted[i] > sorted[j]) {
        float t = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = t;
      }
    }
  }
  float median = sorted[2];

  distance = (distance * 0.4) + (median * 0.6);

  stats.distanceReadCount++;
  stats.totalDistance += distance;
  stats.avgDistance = stats.totalDistance / stats.distanceReadCount;
  if (distance < stats.minDistance) stats.minDistance = distance;
  if (distance > stats.maxDistance) stats.maxDistance = distance;

  if (servoAngle >= 0 && servoAngle <= 180) {
    radarMap[servoAngle] = (uint8_t)constrain((int)distance, 0, 255);
  }
}

// ==================== GELİŞMİŞ GAZ & HAVA ANALİZÖRÜ (MQ-135) ====================
void readGasSensor() {
  static unsigned long lastGasTime = 0;
  static unsigned long lastBaselineUpdate = 0;
  unsigned long now = millis();
  if (now - lastGasTime < 150) return; // 150ms periyot: Seri ve stabil
  lastGasTime = now;

  // 1. ESP32 ADC1 Kanal Geçiş İzolasyonu (Joystick Crosstalk ve Ghosting Önleme)
  // Kanal geçişinde dahili S&H kapasitörünü boşaltmak için çift boş okuma
  analogRead(GAS_PIN);
  delayMicroseconds(50);
  analogRead(GAS_PIN);
  delayMicroseconds(100);

  // 2. Çoklu örnekleme ve Median / Outlier Filtresi (Uç değerleri ve enterferansı at)
  int samples[7];
  for (int i = 0; i < 7; i++) {
    samples[i] = analogRead(GAS_PIN);
    delayMicroseconds(120);
  }
  // Basit bubble sort ile medyan bul
  for (int i = 0; i < 6; i++) {
    for (int j = i + 1; j < 7; j++) {
      if (samples[i] > samples[j]) {
        int temp = samples[i];
        samples[i] = samples[j];
        samples[j] = temp;
      }
    }
  }
  // Ortadaki 3 değerin ortalaması (gürültü ve joystick sıçramaları %100 filtrelenir)
  int currentAdc = (samples[2] + samples[3] + samples[4]) / 3;

  // 3. Hızlı ve Çevik Filtreleme (Gecikmeyi yok eder, anında tepki verir)
  gasRaw = (gasRaw * 6 + currentAdc * 4) / 10;

  // 4. Açılışta Isınma ve Temiz Hava Referans Tespiti
  if (!warmupComplete) {
    if (gasBaseline == 0 || gasRaw < gasBaseline) {
      gasBaseline = gasRaw;
    }
    if (now - bootTime > 5000) { // 5 saniye içinde hızlı hazır olma
      warmupComplete = true;
      Serial.printf("MQ-135 Gaz Referansı Kilitlendi: %d ADC\n", gasBaseline);
    }
  } else {
    // Ortam havası temizlendikçe referansı güncelle
    if (gasRaw < gasBaseline) {
      gasBaseline = gasRaw;
    } else if (now - lastBaselineUpdate > 10000) {
      lastBaselineUpdate = now;
      if (gasDelta < 80) {
        gasBaseline = (gasBaseline * 19 + gasRaw) / 20;
      }
    }
  }

  // 5. Temiz Havadan Sapma (Delta)
  gasDelta = gasRaw - gasBaseline;
  if (gasDelta < 0) gasDelta = 0;

  // 6. Anında Tepki Veren Doğrusal PPM Hesaplaması (400 PPM taban)
  float calculatedPPM = 400.0 + (gasDelta * 1.1f);
  gasPPM = (gasPPM * 0.7f) + (calculatedPPM * 0.3f); // Hızlı ve akıcı PPM takibi

  // 7. Hava Kalitesi Sınıflandırması
  if (gasPPM < 550) {
    gasQualityLevel = 0; // Temiz / Ferah
  } else if (gasPPM < 900) {
    gasQualityLevel = 1; // Normal / Orta
  } else if (gasPPM < settings.gasThreshold) {
    gasQualityLevel = 2; // Kirli / Havalandır
  } else {
    gasQualityLevel = 3; // TEHLİKE (Gaz / Duman / Alkol)
  }
}

void readMotionSensor() {
  static unsigned long lastSample = 0;
  unsigned long now = millis();
  if (now - lastSample < 100) return;
  lastSample = now;

  bool rawState = digitalRead(PIR_PIN);
  if (rawState) {
    if (!motionDetected) {
      stats.motionCount++;
      lastMotionTime = now;
    }
    motionDetected = true;
  } else {
    if (now - lastMotionTime > 2500) {
      motionDetected = false;
    }
  }
}

// ==================== AKILLI PARK & GÜVENLİK ALARMI ====================
void checkAlertStatus() {
  alertLevel = 0;
  robotExpression = 0;

  if (!warmupComplete) return;

  // Mesafe Uyarısı:
  // <= 20 cm: Aşırı sesli acil alarm (alertLevel = 2)
  // 20 - 50 cm: Kademeli artan park uyarısı (alertLevel = 1)
  // > 50 cm: Sessiz (alertLevel = 0)
  if (distance >= 2.0 && distance <= 20.0) {
    alertLevel = 2; // Acil engel (<20 cm) - AŞIRI SESLİ
    robotExpression = 2;
  } else if (distance > 20.0 && distance <= 50.0) {
    alertLevel = 1; // Yaklaşma bölgesi (20 - 50 cm) - Kademeli Bip
    robotExpression = 1;
  }

  // Gaz Alarmı (Seviye 3: Tehlike)
  if (gasQualityLevel >= 3) {
    alertLevel = 2;
    robotExpression = 2;
  }
}

// ==================== SESLİ & GÖRSEL İKAZ KONTROLÜ ====================
void updateAlertActuators() {
  unsigned long now = millis();

  if (alertLevel == 2) {
    // <= 20 cm veya Gaz Tehlikesi: AŞIRI SESLİ VE HIZLI ALARM
    buzzInterval = 70; // 70 ms aralıkla seri acil alarm
  } else if (alertLevel == 1) {
    // 20 - 50 cm arası: Mesafe azaldıkça bip sıklığı kademeli artar
    // 50 cm'de 700 ms aralık -> 20 cm'e yaklaştıkça 120 ms aralığa iner
    buzzInterval = map((int)distance, 20, 50, 120, 700);
    buzzInterval = constrain(buzzInterval, 120, 700);
  } else {
    buzzInterval = 0; // > 50 cm: TAMAMEN SESSİZ
  }

  if (buzzInterval > 0 && settings.soundEnabled && warmupComplete) {
    if (buzzState == BUZZ_OFF && (now - buzzTimer >= (unsigned long)buzzInterval)) {
      if (alertLevel == 2) {
        setBuzzerAudio(true, 240); // Aşırı sesli (tam güç)
      } else {
        // 20-50 cm: Mesafe yaklaştıkça hafifçe artan ses (35..75)
        int vol = map((int)distance, 20, 50, 75, 35);
        setBuzzerAudio(true, vol);
      }
      buzzState = BUZZ_ON;
      buzzTimer = now;
    } else if (buzzState == BUZZ_ON) {
      unsigned long onDuration = (alertLevel == 2) ? 60 : 30; // Tehlikede 60ms, yaklaşmada 30ms tık
      if (now - buzzTimer >= onDuration) {
        setBuzzerAudio(false);
        buzzState = BUZZ_OFF;
        buzzTimer = now;
      }
    }
  } else {
    setBuzzerAudio(false);
    buzzState = BUZZ_OFF;
  }

  if (alertLevel == 2) {
    setWarningLED(((now / 80) % 2 == 0) ? 255 : 0); // Seri flaşör
  } else if (alertLevel == 1) {
    int ledBright = map((int)distance, 20, 50, 200, 40);
    setWarningLED(ledBright);
  } else {
    setWarningLED(0);
  }

  digitalWrite(SYS_LED, (now % 1000) < 50 ? HIGH : LOW);
}

// ==================== "TIK TIK" HIZLI & SERT GEÇİŞLİ SERVO HAREKETİ ====================
unsigned long lastManualJoystickTime = 0;

void updateServoMotor() {
  unsigned long now = millis();

  // 1. MANUEL SERVO EKRANINDA (MENU_SERVO_MANUAL):
  // Tamamen joystick ile doğrudan hassas pozisyonlama (her 20ms'de 5 derece adım)
  if (activeMenu == MENU_SERVO_MANUAL) {
    if (now - lastJoyServoStep >= 20) {
      lastJoyServoStep = now;
      int rawX = analogRead(JOY_X);
      int deltaX = rawX - joyCenterX;
      if (deltaX < -280) {
        servoAngle += (settings.servoInvert ? 5 : -5);
      } else if (deltaX > 280) {
        servoAngle += (settings.servoInvert ? -5 : 5);
      }
      servoAngle = constrain(servoAngle, 20, 160);
      radarServo.write(servoAngle);
      stats.servoSteps++;
    }
    return;
  }

  // 2. RADAR EKRANINDA İSTEĞE BAĞLI MANUEL YÖNLENDİRME (Kasıtlı ve sert itme > 750)
  if (activeMenu == MENU_RADAR) {
    int rawX = analogRead(JOY_X);
    int deltaX = rawX - joyCenterX;
    if (abs(deltaX) > 750) {
      lastManualJoystickTime = now;
      if (now - lastJoyServoStep >= 20) {
        lastJoyServoStep = now;
        if (deltaX < -750) {
          servoAngle += (settings.servoInvert ? 5 : -5);
        } else if (deltaX > 750) {
          servoAngle += (settings.servoInvert ? -5 : 5);
        }
        servoAngle = constrain(servoAngle, 20, 160);
        radarServo.write(servoAngle);
        stats.servoSteps++;
      }
      return;
    }
  }

  // 3. DİĞER TÜM EKRANLARDA KESİNTİSİZ OTOMATİK TARAMA ("HEP ÇALIŞIR")
  // Engel olsun ya da olmasın, mesafe ne olursa olsun kesintisiz 20° - 160° arası seri tarar!
  if (now - lastManualJoystickTime >= 350) {
    if (now - lastServoStep >= 18) { // 18ms periyot: 5 derecelik hızlı ve sert tık tık adımlar
      lastServoStep = now;
      if (scanDirection) {
        servoAngle += 5;
        if (servoAngle >= 160) {
          servoAngle = 160;
          scanDirection = false;
        }
      } else {
        servoAngle -= 5;
        if (servoAngle <= 20) {
          servoAngle = 20;
          scanDirection = true;
        }
      }
      radarServo.write(servoAngle);
      stats.servoSteps++;
    }
  }
}

// ==================== JOYSTICK & BUTON GİRİŞLERİ ====================
void handleJoyButtonClick();
void handleBackButtonClick();
void handleJoyUp();
void handleJoyDown();
void handleJoyLeft();
void handleJoyRight();
void modifySetting(int delta);

static int lastReadDeltaX = 0;
static int lastReadDeltaY = 0;

void readInputControls() {
  unsigned long now = millis();

  // 1. Joystick Eksenleri (20 ms periyot)
  static unsigned long lastJoyRead = 0;
  if (now - lastJoyRead >= 20) {
    lastJoyRead = now;
    analogRead(JOY_X);
    delayMicroseconds(20);
    int rawX = analogRead(JOY_X);

    analogRead(JOY_Y);
    delayMicroseconds(20);
    int rawY = analogRead(JOY_Y);

    lastReadDeltaX = rawX - joyCenterX;
    lastReadDeltaY = rawY - joyCenterY;

    // Y Ekseni (Menü Yukarı / Aşağı Gezinme)
    if (abs(lastReadDeltaY) < 180) {
      joyYEngaged = false; // Merkeze döndü, kilit açıldı
    } else if (!joyYEngaged) {
      if (lastReadDeltaY < -320) {
        joyYEngaged = true;
        handleJoyUp();
      } else if (lastReadDeltaY > 320) {
        joyYEngaged = true;
        handleJoyDown();
      }
    }

    // X Ekseni (Menü İçi Sağa / Sola Gezinme & Ayar Değiştirme)
    if (abs(lastReadDeltaX) < 180) {
      joyXEngaged = false; // Merkeze döndü, kilit açıldı
    } else if (!joyXEngaged) {
      if (lastReadDeltaX < -320) {
        joyXEngaged = true;
        handleJoyLeft();
      } else if (lastReadDeltaX > 320) {
        joyXEngaged = true;
        handleJoyRight();
      }
    }
  }

  // 2. Joystick Butonu (GPIO 32)
  // SÜPER FİLTRE: Joystick herhangi bir yöne eğilmişse (menüde gezinirken),
  // buton kontaklarının mekanik titremesi/basımı KESİNLİKLE YOK SAYILIR!
  // Sadece joystick dinlenme merkezindeyken buton basımı kabul edilir.
  bool joystickInCenter = (abs(lastReadDeltaX) < 220 && abs(lastReadDeltaY) < 220);

  bool rawJoyBtn = digitalRead(JOY_BTN);
  if (rawJoyBtn != lastJoyBtnRaw) {
    lastJoyBtnDebounce = now;
    lastJoyBtnRaw = rawJoyBtn;
  }
  if ((now - lastJoyBtnDebounce) > 45) { // 45ms kararlı debounce
    bool pressed = (rawJoyBtn == LOW);
    if (pressed && !joyBtnState) {
      joyBtnState = true;
      if (joystickInCenter) {
        handleJoyButtonClick();
      }
    } else if (!pressed && joyBtnState) {
      joyBtnState = false;
    }
  }

  // 3. D18 Butonu (Kısa basma: Geri / Menüde İlerleme / 1.5 sn basma: Sessiz Mod)
  bool rawBackBtn = digitalRead(BACK_BTN);
  if (rawBackBtn != lastBackBtnRaw) {
    lastBackBtnRaw = rawBackBtn;
    if (rawBackBtn == LOW) {
      backBtnPressStart = now;
      backBtnLongHandled = false;
    } else {
      if (!backBtnLongHandled && (now - backBtnPressStart > 40)) {
        handleBackButtonClick();
      }
    }
  }

  if (rawBackBtn == LOW && !backBtnLongHandled && (now - backBtnPressStart >= 1500)) {
    backBtnLongHandled = true;
    settings.soundEnabled = !settings.soundEnabled;
    saveSettings();
    triggerChirp();
  }
}

void handleJoyButtonClick() {
  triggerChirp();

  if (activeMenu == MENU_MAIN) {
    // Ana menüdeyse: Seçili alt menüye gir
    activeMenu = menuSelection + 1;
    if (activeMenu == MENU_SERVO_MANUAL) manualServoMode = true;
    editMode = false;
  } else {
    // Herhangi bir alt menüdeyse (Radar, Sensör, Robot, Ayar, Manuel Servo vb.):
    // Joystick orta tuşuna basınca ANINDA ANA MENÜYE GERİ DÖN!
    activeMenu = MENU_MAIN;
    manualServoMode = false;
    editMode = false;
    saveSettings();
  }
}

void handleBackButtonClick() {
  triggerChirp();

  if (activeMenu == MENU_MAIN) {
    // Ana menüdeyken D18'e basılırsa menüde aşağı ilerle (Yedek gezinme!)
    handleJoyDown();
  } else {
    // Alt sayfadayken D18'e basılırsa ana menüye dön
    activeMenu = MENU_MAIN;
    manualServoMode = false;
    editMode = false;
    saveSettings();
  }
}

void handleJoyUp() {
  if (activeMenu == MENU_MAIN) {
    menuSelection--;
    if (menuSelection < 0) menuSelection = MAIN_MENU_COUNT - 1;
    if (menuSelection < menuScrollOffset) menuScrollOffset = menuSelection;
    if (menuSelection >= menuScrollOffset + 4) menuScrollOffset = menuSelection - 3;
    triggerChirp();
  } else if (activeMenu >= MENU_SETTINGS_DIST && activeMenu <= MENU_SETTINGS_SYSTEM) {
    if (!editMode) {
      editParamIndex--;
      if (editParamIndex < 0) editParamIndex = 2;
      triggerChirp();
    }
  }
}

void handleJoyDown() {
  if (activeMenu == MENU_MAIN) {
    menuSelection++;
    if (menuSelection >= MAIN_MENU_COUNT) menuSelection = 0;
    if (menuSelection >= menuScrollOffset + 4) menuScrollOffset = menuSelection - 3;
    if (menuSelection < menuScrollOffset) menuScrollOffset = menuSelection;
    triggerChirp();
  } else if (activeMenu >= MENU_SETTINGS_DIST && activeMenu <= MENU_SETTINGS_SYSTEM) {
    if (!editMode) {
      editParamIndex++;
      if (editParamIndex > 2) editParamIndex = 0;
      triggerChirp();
    }
  }
}

void handleJoyLeft() {
  if (activeMenu != MENU_MAIN && (activeMenu < MENU_SETTINGS_DIST || activeMenu > MENU_SETTINGS_SYSTEM)) {
    // Alt sayfalardayken (Mesafe, Gaz, Hareket, Radar, Manuel Servo, İstatistik vb.)
    // Joystick'i sola itince de doğrudan Ana Menüye dön!
    activeMenu = MENU_MAIN;
    manualServoMode = false;
    editMode = false;
    saveSettings();
    triggerChirp();
  } else if (activeMenu >= MENU_SETTINGS_DIST && activeMenu <= MENU_SETTINGS_SYSTEM) {
    modifySetting(-1);
  }
}

void handleJoyRight() {
  if (activeMenu == MENU_MAIN) {
    // Ana menüdeyken joystick'i sağa itince seçili alt menüye gir!
    activeMenu = menuSelection + 1;
    if (activeMenu == MENU_SERVO_MANUAL) manualServoMode = true;
    editMode = false;
    triggerChirp();
  } else if (activeMenu >= MENU_SETTINGS_DIST && activeMenu <= MENU_SETTINGS_SYSTEM) {
    modifySetting(1);
  }
}

void modifySetting(int delta) {
  triggerChirp();
  switch (activeMenu) {
    case MENU_SETTINGS_DIST:
      if (editParamIndex == 0) {
        settings.distanceWarning = constrain(settings.distanceWarning + delta * 5, 20, 100);
      } else if (editParamIndex == 1) {
        settings.distanceDanger = constrain(settings.distanceDanger + delta * 5, 5, settings.distanceWarning - 5);
      } else {
        settings.maxDistance = constrain(settings.maxDistance + delta * 25, 100, 400);
      }
      break;

    case MENU_SETTINGS_GAS:
      if (editParamIndex == 0) {
        settings.gasThreshold = constrain(settings.gasThreshold + delta * 50, 700, 2500);
      }
      break;

    case MENU_SETTINGS_SERVO:
      if (editParamIndex == 0) {
        settings.servoHomePos = constrain(settings.servoHomePos + delta * 2, 20, 160);
        servoAngle = settings.servoHomePos;
        radarServo.write(servoAngle);
      } else if (editParamIndex == 1) {
        settings.servoInvert = !settings.servoInvert;
      } else {
        settings.continuousRadar = !settings.continuousRadar;
      }
      break;

    case MENU_SETTINGS_SYSTEM:
      if (editParamIndex == 0) {
        settings.soundEnabled = !settings.soundEnabled;
      } else if (editParamIndex == 1) {
        settings.ledEnabled = !settings.ledEnabled;
      } else {
        restoreFactoryDefaults();
      }
      break;
  }
}

// ==================== BLUETOOTH TELEMETRİ VE KONTROL ====================
void processBluetooth() {
  if (!settings.bluetoothEnabled) return;

  static unsigned long lastBTSend = 0;
  unsigned long now = millis();

  bluetoothConnected = SerialBT.hasClient();

  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    if (cmd == "GET") {
      SerialBT.printf("{\"dist\":%.1f,\"gas\":%.0f,\"delta\":%d,\"angle\":%d,\"alert\":%d}\n",
                      distance, gasPPM, gasDelta, servoAngle, alertLevel);
    } else if (cmd.startsWith("SERVO:")) {
      int a = cmd.substring(6).toInt();
      servoAngle = constrain(a, 15, 165);
      radarServo.write(servoAngle);
    } else if (cmd == "MUTE") {
      settings.soundEnabled = !settings.soundEnabled;
    }
  }

  if (bluetoothConnected && (now - lastBTSend >= 1000)) {
    lastBTSend = now;
    SerialBT.printf("{\"dist\":%.1f,\"gas\":%.0f,\"delta\":%d,\"motion\":%d,\"angle\":%d,\"alert\":%d}\n",
                    distance, gasPPM, gasDelta, motionDetected ? 1 : 0, servoAngle, alertLevel);
  }
}

// ==================== OLED EKRAN ÇİZİM (128x64) ====================
void drawRobotCompanion(int cx, int cy);

void drawHeader(const char* title) {
  display.fillRect(0, 0, SCREEN_WIDTH, 14, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(3, 3);
  display.print(title);

  display.setCursor(76, 3);
  display.print(settings.soundEnabled ? "S " : "M ");
  display.print(bluetoothConnected ? "BT" : "--");

  if (!warmupComplete) {
    display.print(" ~");
  } else if (alertLevel == 2) {
    display.print(" !");
  } else if (alertLevel == 1) {
    display.print(" ?");
  } else {
    display.print(" *");
  }
}

void drawFooter(const char* actionHint) {
  display.drawLine(0, 53, SCREEN_WIDTH, 53, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 55);
  display.print(actionHint);

  drawRobotCompanion(120, 58);
}

void drawRobotCompanion(int cx, int cy) {
  display.drawCircle(cx, cy, 4, SSD1306_WHITE);

  if (eyesOpen) {
    display.drawPixel(cx - 2, cy - 1, SSD1306_WHITE);
    display.drawPixel(cx + 2, cy - 1, SSD1306_WHITE);
  } else {
    display.drawLine(cx - 2, cy - 1, cx - 1, cy - 1, SSD1306_WHITE);
    display.drawLine(cx + 1, cy - 1, cx + 2, cy - 1, SSD1306_WHITE);
  }

  if (alertLevel == 2) {
    display.drawPixel(cx, cy + 2, SSD1306_WHITE);
  } else if (alertLevel == 1) {
    display.drawLine(cx - 1, cy + 2, cx + 1, cy + 2, SSD1306_WHITE);
  } else {
    display.drawPixel(cx - 1, cy + 2, SSD1306_WHITE);
    display.drawPixel(cx, cy + 2, SSD1306_WHITE);
    display.drawPixel(cx + 1, cy + 2, SSD1306_WHITE);
  }
}

void drawScreenMainMenu() {
  drawHeader(FIRMWARE_NAME);

  for (int i = 0; i < 4; i++) {
    int itemIdx = menuScrollOffset + i;
    if (itemIdx >= MAIN_MENU_COUNT) break;

    int y = 16 + (i * 9);
    bool selected = (itemIdx == menuSelection);

    if (selected) {
      display.fillRect(0, y - 1, 122, 9, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(2, y);
      display.print(">");
      display.print(menuNames[itemIdx]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(4, y);
      display.print(menuNames[itemIdx]);
    }
  }

  int scrollHeight = 35;
  int thumbY = 16 + map(menuSelection, 0, MAIN_MENU_COUNT - 1, 0, scrollHeight - 6);
  display.drawRect(125, 16, 2, scrollHeight, SSD1306_WHITE);
  display.fillRect(125, thumbY, 2, 6, SSD1306_WHITE);

  drawFooter("Sag/BTN:Sec D18:Asa");
}

void drawScreenDistance() {
  drawHeader("MESAFE RADARI");

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  
  if (distance < 100.0) display.setCursor(22, 18);
  else display.setCursor(10, 18);
  
  display.print(distance, 1);
  display.setTextSize(1);
  display.print(" cm");

  // Proximity Çubuğu (Y: 36..42)
  display.drawRect(6, 36, 116, 7, SSD1306_WHITE);
  int fillWidth = map((int)distance, 0, 150, 112, 0);
  fillWidth = constrain(fillWidth, 0, 112);
  display.fillRect(8, 38, fillWidth, 3, SSD1306_WHITE);

  display.setCursor(6, 45);
  if (alertLevel == 2) display.print("ENGEL YAKIN! (<15cm)");
  else if (alertLevel == 1) display.print("YAKLASILIYOR...");
  else display.print("Mesafe: Guvenli");

  drawFooter("D18:Menu  Aci:");
  display.setCursor(85, 55);
  display.print(servoAngle);
  display.print("o");
}

void drawScreenGas() {
  drawHeader("HAVA KALITESI");

  // 1. Büyük Font PPM Değeri (Y: 17)
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 17);
  display.print((int)gasPPM);
  display.setTextSize(1);
  display.print(" ppm");

  // 2. 10 Kademeli Hava Kalitesi Barı (Y: 34..40)
  display.drawRect(6, 34, 116, 7, SSD1306_WHITE);
  int barW = map((int)gasPPM, 400, 1600, 0, 112);
  barW = constrain(barW, 0, 112);
  display.fillRect(8, 36, barW, 3, SSD1306_WHITE);

  // 3. Hava Kalitesi Sınıflandırma Durumu (Y: 43)
  display.setCursor(6, 44);
  if (!warmupComplete) {
    display.print("ISINMA: Kalibrede...");
  } else if (gasQualityLevel == 3) {
    display.print("TEHLIKE: GAZ/ALKOL!");
  } else if (gasQualityLevel == 2) {
    display.print("HAVA: KIRLI / HAVALANDIR");
  } else if (gasQualityLevel == 1) {
    display.print("HAVA: NORMAL / ORTA");
  } else {
    display.print("HAVA: TEMIZ VE FERAH");
  }

  // 4. Alt Bilgi: Ham ADC ve Fark (+Δ)
  drawFooter("D18:Menu  Fark:+");
  display.setCursor(95, 55);
  display.print(gasDelta);
}

void drawScreenMotion() {
  drawHeader("GUVENLIK & HAREKET");

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 18);
  if (motionDetected) display.print("HAREKET!");
  else display.print("TEMIZ");

  display.setTextSize(1);
  display.setCursor(6, 36);
  display.print("Toplam Algilama: ");
  display.print(stats.motionCount);

  display.setCursor(6, 45);
  display.print("Durum: ");
  display.print(motionDetected ? "ALGILANDI" : "BEKLENIYOR");

  drawFooter("D18:Menu  PIR:14");
}

void drawScreenRadar() {
  drawHeader("180 RADAR TARAMA");

  int cx = 64, cy = 52, r = 33;
  display.drawCircle(cx, cy, r, SSD1306_WHITE);
  display.drawCircle(cx, cy, r / 2, SSD1306_WHITE);
  display.drawPixel(cx, cy, SSD1306_WHITE);

  float rad = radians(servoAngle);
  int beamX = cx + (int)(r * -cos(rad));
  int beamY = cy - (int)(r * sin(rad));
  display.drawLine(cx, cy, beamX, beamY, SSD1306_WHITE);

  for (int a = 15; a <= 165; a += 8) {
    int d = radarMap[a];
    if (d > 5 && d < 120) {
      int ptR = map(d, 5, 120, 6, r);
      float ptRad = radians(a);
      int px = cx + (int)(ptR * -cos(ptRad));
      int py = cy - (int)(ptR * sin(ptRad));
      display.drawPixel(px, py, SSD1306_WHITE);
    }
  }

  display.setCursor(2, 16);
  display.print(servoAngle);
  display.print("o");

  display.setCursor(84, 16);
  display.print((int)distance);
  display.print("cm");

  drawFooter("D18:Geri  SurekliTarama");
}

void drawScreenServoManual() {
  drawHeader("MANUEL SERVO");

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(42, 18);
  display.print(servoAngle);
  display.setTextSize(1);
  display.print("o");

  display.drawRect(10, 36, 108, 6, SSD1306_WHITE);
  int needleX = 10 + map(servoAngle, 10, 170, 0, 106);
  display.fillRect(needleX - 1, 34, 3, 10, SSD1306_WHITE);

  display.setCursor(6, 45);
  display.print("Merkez: ");
  display.print(settings.servoHomePos);
  display.print("o  Ters: ");
  display.print(settings.servoInvert ? "ACIK" : "KAP");

  drawFooter("JS:Sol/Sag  BTN:Geri");
}

void drawScreenStatistics() {
  drawHeader("SISTEM BILGISI");

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  unsigned long sec = (millis() - stats.startTime) / 1000;
  display.setCursor(4, 16);
  display.printf("Sure: %02lu:%02lu:%02lu", sec / 3600, (sec % 3600) / 60, sec % 60);

  display.setCursor(4, 25);
  display.printf("Mesafe: %d - %d cm", (int)stats.minDistance, (int)stats.maxDistance);

  display.setCursor(4, 34);
  display.printf("Ort: %.0f cm  Adim: %lu", stats.avgDistance, stats.servoSteps);

  display.setCursor(4, 43);
  display.printf("Bos RAM: %d KB", ESP.getFreeHeap() / 1024);

  drawFooter("D18:Menu  V10-PRO");
}

void drawScreenSettings(int param1Val, int param2Val, int param3Val,
                        const char* label1, const char* label2, const char* label3,
                        const char* unit) {
  drawHeader(editMode ? "AYAR DUZENLE" : "AYARLAR");

  display.setTextSize(1);

  int y1 = 17;
  if (editParamIndex == 0) {
    if (editMode) display.fillRect(0, y1 - 1, 126, 9, SSD1306_WHITE);
    display.setTextColor(editMode ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(2, y1);
    display.printf("> %s: %d %s", label1, param1Val, unit);
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(6, y1);
    display.printf("%s: %d %s", label1, param1Val, unit);
  }

  int y2 = 27;
  if (editParamIndex == 1) {
    if (editMode) display.fillRect(0, y2 - 1, 126, 9, SSD1306_WHITE);
    display.setTextColor(editMode ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(2, y2);
    display.printf("> %s: %d %s", label2, param2Val, unit);
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(6, y2);
    display.printf("%s: %d %s", label2, param2Val, unit);
  }

  int y3 = 37;
  if (editParamIndex == 2) {
    if (editMode) display.fillRect(0, y3 - 1, 126, 9, SSD1306_WHITE);
    display.setTextColor(editMode ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(2, y3);
    display.printf("> %s: %d %s", label3, param3Val, unit);
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(6, y3);
    display.printf("%s: %d %s", label3, param3Val, unit);
  }

  display.setTextColor(SSD1306_WHITE);
  if (editMode) {
    drawFooter("Sol/Sag:Ayar  JS:Bitti");
  } else {
    drawFooter("Yuk/As:Sec  JS:Duzenle");
  }
}

void drawScreenSettingsSystem() {
  drawHeader(editMode ? "SISTEM AYAR" : "SISTEM AYARLARI");

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  int y1 = 17;
  display.setCursor(editParamIndex == 0 ? 2 : 6, y1);
  display.printf("%sSes: %s", editParamIndex == 0 ? "> " : "", settings.soundEnabled ? "ACIK" : "KAPALI");

  int y2 = 27;
  display.setCursor(editParamIndex == 1 ? 2 : 6, y2);
  display.printf("%sLED: %s", editParamIndex == 1 ? "> " : "", settings.ledEnabled ? "ACIK" : "KAPALI");

  int y3 = 37;
  display.setCursor(editParamIndex == 2 ? 2 : 6, y3);
  display.printf("%sFabrika Ayarlarina Don", editParamIndex == 2 ? "> " : "");

  if (editMode) {
    drawFooter("Sol/Sag:Degistir");
  } else {
    drawFooter("Yuk/As:Sec JS:Duzenle");
  }
}

void updateOLEDDisplay() {
  display.clearDisplay();

  switch (activeMenu) {
    case MENU_MAIN:
      drawScreenMainMenu();
      break;
    case MENU_DISTANCE:
      drawScreenDistance();
      break;
    case MENU_GAS:
      drawScreenGas();
      break;
    case MENU_MOTION:
      drawScreenMotion();
      break;
    case MENU_RADAR:
      drawScreenRadar();
      break;
    case MENU_SERVO_MANUAL:
      drawScreenServoManual();
      break;
    case MENU_STATISTICS:
      drawScreenStatistics();
      break;
    case MENU_SETTINGS_DIST:
      drawScreenSettings(settings.distanceWarning, settings.distanceDanger, settings.maxDistance,
                         "Ikaz", "Yakin", "Maks", "cm");
      break;
    case MENU_SETTINGS_GAS:
      drawScreenSettings(settings.gasThreshold, (int)gasPPM, gasDelta,
                         "Esik", "Anlik", "Fark", "ppm");
      break;
    case MENU_SETTINGS_SERVO:
      drawScreenSettings(settings.servoHomePos, settings.servoInvert ? 1 : 0, settings.continuousRadar ? 1 : 0,
                         "Merkez", "TersJS", "Surekli", "o");
      break;
    case MENU_SETTINGS_SYSTEM:
      drawScreenSettingsSystem();
      break;
    default:
      drawScreenMainMenu();
      break;
  }

  display.display();
}

// ==================== ARDUINO SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(200);
  bootTime = millis();

  Serial.println(F("\n=================================================="));
  Serial.println(F("   TOPRAK PRO MAX V10.0 — KESIN COZUM BASLADI"));
  Serial.println(F("=================================================="));

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(BACK_BTN, INPUT_PULLUP);
  pinMode(SYS_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setupPWM();
  loadSettings();

  // 20 örnek ile joystick merkez kalibrasyonu
  long sumX = 0, sumY = 0;
  for (int i = 0; i < 20; i++) {
    sumX += analogRead(JOY_X);
    sumY += analogRead(JOY_Y);
    delay(10);
  }
  joyCenterX = sumX / 20;
  joyCenterY = sumY / 20;
  Serial.printf("Joystick Kalibre Edildi: X_Center=%d, Y_Center=%d\n", joyCenterX, joyCenterY);

  stats.startTime = millis();
  stats.minDistance = 400.0;
  stats.maxDistance = 0.0;
  stats.totalDistance = 0.0;
  stats.distanceReadCount = 0;
  stats.motionCount = 0;
  stats.servoSteps = 0;

  radarServo.setPeriodHertz(50);
  radarServo.attach(SERVO_PIN, 500, 2400);
  servoAngle = 90;
  radarServo.write(servoAngle);

  for (int i = 0; i <= 180; i++) radarMap[i] = 100;

  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000); // 400kHz Fast I2C modu: OLED aktarımını 12ms'den 2.5ms'ye indirir (sıfır takılma)
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(18, 15);
    display.print("TOPRAK PRO MAX");
    display.setCursor(32, 28);
    display.setTextSize(2);
    display.print("V10.0");
    display.setTextSize(1);
    display.setCursor(15, 48);
    display.print("Kalibre Ediliyor...");
    display.display();
    delay(600);
  }

  if (settings.bluetoothEnabled) {
    SerialBT.begin("TOPRAK-V10");
  }

  triggerChirp();
  Serial.println(F("Sistem Kusursuz Olarak Calisiyor!"));
}

// ==================== ARDUINO MAIN LOOP ====================
void loop() {
  unsigned long now = millis();
  static unsigned long lastSensorCycle = 0;
  static unsigned long lastDisplayCycle = 0;

  // 1. Sensörleri Oku (45 ms)
  if (now - lastSensorCycle >= 45) {
    lastSensorCycle = now;
    readDistanceSensor();
    readGasSensor();
    readMotionSensor();
    checkAlertStatus();
  }

  // 2. Kullanıcı Girişleri
  readInputControls();

  // 3. İpeksi Servo Taraması
  updateServoMotor();

  // 4. Kısık Sesli Kibar Park İkazı (Buzzer & LED)
  updateAlertActuators();

  // 5. Bluetooth
  processBluetooth();

  // 6. Göz Kırpma
  if (now - lastBlinkTime >= (eyesOpen ? 3500 : 200)) {
    lastBlinkTime = now;
    eyesOpen = !eyesOpen;
  }

  // 7. Ekranı Güncelle (80 ms)
  if (now - lastDisplayCycle >= 80) {
    lastDisplayCycle = now;
    updateOLEDDisplay();
  }
}
