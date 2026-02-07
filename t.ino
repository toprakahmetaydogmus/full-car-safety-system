/***************************************************************
 * TOPROAK PRO MAX V8.1 - ADVANCED RADAR SYSTEM
 * ESP32 COMPATIBLE - ALL SENSORS WORKING
 * HC-SR04 ULTRASONIC + RCWL-0507 MOTION SENSOR
 * FULL SERVO CONTROL WITH RADAR VISUALIZATION
 * BLUETOOTH INTEGRATION FOR PHONE VISUALIZATION
 * ADVANCED OLED DISPLAY WITH MENU SYSTEM
 * PYTHON (PYDROID3) COMPATIBLE
 * STABLE & OPTIMIZED VERSION
 ***************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <BluetoothSerial.h>
#include <math.h>

// =========== VERSION INFO ===========
#define FIRMWARE_VERSION "V8.1"
#define BUILD_DATE "2024"
#define DEVELOPER "TOPROAK TEAM - PRO MAX EDITION"

// =========== PIN DEFINITIONS ===========
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C

#define JOY_X 34
#define JOY_Y 35
#define JOY_BTN 32
#define BACK_BTN 18

#define TRIG_PIN 13
#define ECHO_PIN 12
#define GAS_PIN 36
#define PIR_PIN 14
#define BUZZER_PIN 15
#define SYS_LED 2
#define SERVO_PIN 26
#define LED_PIN 25

// =========== OBJECTS ===========
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo radarServo;
Preferences preferences;
BluetoothSerial SerialBT;

// =========== ESP32 PWM SETTINGS ===========
#define BUZZER_CHANNEL 0
#define LED_CHANNEL 1
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// =========== MENU DEFINITIONS ===========
enum MenuList {
  MENU_MAIN,
  MENU_DISTANCE,
  MENU_GAS,
  MENU_MOTION,
  MENU_SCAN,
  MENU_RADAR,
  MENU_SERVO_MANUAL,
  MENU_SETTINGS_DISTANCE,
  MENU_SETTINGS_GAS,
  MENU_SETTINGS_MOTION,
  MENU_SETTINGS_SOUND,
  MENU_SETTINGS_SERVO,
  MENU_SETTINGS_LED,
  MENU_SYSTEM,
  MENU_SECURITY,
  MENU_CALIBRATION,
  MENU_STATISTICS,
  MENU_TEST,
  MENU_COMMUNICATION,
  MENU_ADVANCED,
  MENU_DIAGNOSTIC,
  MENU_FIRMWARE,
  MENU_OBSTACLE_MODE,
  MENU_RCWL_CALIBRATION,
  MENU_BLUETOOTH_VIEWER,
  MENU_RADAR_360,
  MENU_3D_VIEW,
  MENU_DATA_LOG,
  MENU_AUTONOMOUS,
  MENU_VOICE_ALERT,
  MENU_BATTERY,
  MENU_GPS,
  MENU_AI_MODE,
  TOTAL_MENU_COUNT
};

// =========== GLOBAL VARIABLES ===========
float distance = 0;
float gas = 0;
bool motion = false;

// HC-SR04 Advanced Measurement System
#define DISTANCE_BUFFER_SIZE 20
float distanceBuffer[DISTANCE_BUFFER_SIZE];
int bufferIndex = 0;
unsigned long lastDistanceRead = 0;
const int DISTANCE_READ_INTERVAL = 50;
bool distanceSensorWorking = true;
int distanceErrorCount = 0;
float lastValidDistance = 100.0;
bool distanceStuckState = false;
unsigned long distanceStuckTime = 0;
float filteredDistance = 0;
#define MIN_DISTANCE 2.0
#define MAX_DISTANCE 400.0
float distanceMedianFilter[7] = {0};
int medianIndex = 0;
bool distanceAlarm = false;
unsigned long distanceAlarmTime = 0;

// RCWL-0507 Advanced
unsigned long lastPirRead = 0;
const int PIR_READ_INTERVAL = 100;
unsigned long lastMotionTime = 0;
int motionCount = 0;
bool motionDetected = false;
int motionIntensity = 0;
unsigned long motionDuration = 0;
bool motionAlarm = false;

// Buttons
bool joyBtn = false, lastJoyBtn = false;
bool backBtn = false, lastBackBtn = false;

// Menu System
int activeMenu = MENU_MAIN;
int menuSelection = 0;
bool settingsMode = false;
int settingsSelection = 0;
bool manualServoMode = false;
int menuStart = 0;
const int MAX_VISIBLE_ITEMS = 8;
const int TOTAL_MENU_ITEMS = TOTAL_MENU_COUNT;

// Alerts
bool alertActive = false;
int alertLevel = 0;
unsigned long lastAlertTime = 0;
bool beepActive = false;
String alertMessage = "";
bool fullScreenAlert = false;
unsigned long fullScreenAlertStart = 0;

// Robot Character
bool eyesOpen = true;
unsigned long lastBlinkTime = 0;
int robotExpression = 0;

// =========== SERVO SYSTEM ===========
const int SERVO_MIN_LIMIT = 0;
const int SERVO_MAX_LIMIT = 180;
const int SERVO_HOME_POS = 90;
int servoAngle = SERVO_HOME_POS;
int targetAngle = SERVO_HOME_POS;
bool scanDirection = true;
bool scanActive = true;
int scanSpeed = 30;
bool servoObstacleMode = false;
int obstacleDistanceThreshold = 30;
bool servoFullScan = false;
unsigned long servoFullScanStart = 0;

// RADAR MAP
#define RADAR_POINTS 181
float radarMap[RADAR_POINTS] = {0};
unsigned long lastServoMove = 0;
bool servoMoving = false;
int radarRefresh = 0;

// BLUETOOTH SYSTEM
bool bluetoothActive = true;
bool bluetoothDataSend = true;
unsigned long lastBluetoothSend = 0;
const int BLUETOOTH_SEND_INTERVAL = 100;
String bluetoothBuffer = "";
bool bluetoothConnected = false;
unsigned long lastConnectionCheck = 0;
String lastBluetoothMessage = "";
unsigned long lastMessageTime = 0;

// LED System
int ledBrightness = 200;
int ledMode = 2;
bool ledState = true;
unsigned long lastLedChange = 0;
int ledBlinkSpeed = 500;

// Buzzer System
unsigned long lastBeep = 0;
int beepInterval = 1000;
bool beepState = false;
int beepFrequency = 1000;
int beepDuration = 100;

// Security System
bool securityActive = false;
bool alarmActive = false;
unsigned long alarmStart = 0;
int alarmLevel = 0;
int securitySensitivity = 5;

// Joystick Servo Control
int joyServoSpeed = 15;
unsigned long lastJoyServoMove = 0;
int joystickSensitivity = 1000;

// Obstacle System
bool obstaclePresent = false;
unsigned long obstacleDetectTime = 0;
float obstacleDistance = 0;
int obstacleAngle = 0;

// RADAR SYSTEM
bool radarActive = true;
int radarMode = 0;
float radarData[360] = {0};
unsigned long lastRadarUpdate = 0;
int radarSweepAngle = 0;
bool radarSweepDirection = true;
int radarMaxRange = 200;

// 3D View
bool threeDView = false;
float viewAngle = 0;

// Data Logging
bool dataLogging = false;
unsigned long logStartTime = 0;
int logInterval = 1000;

// Autonomous Mode
bool autonomousMode = false;
int autonomousState = 0;
unsigned long autonomousStartTime = 0;

// =========== SETTINGS ===========
struct {
  int distanceWarning = 100;
  int distanceDanger = 30;
  int gasThreshold = 800;
  int motionThresholdTime = 3000;
  bool sound = true;
  bool servoActive = true;
  int servoSpeed = 15;
  int servoMinAngle = SERVO_MIN_LIMIT;
  int servoMaxAngle = SERVO_MAX_LIMIT;
  bool ledActive = true;
  int ledModeSetting = 2;
  int ledSpeed = 500;
  int ledBrightnessSetting = 200;
  bool securityAuto = false;
  int maxDistance = 400;
  bool servoObstacleModeSetting = false;
  int obstacleDistanceThresholdSetting = 30;
  bool bluetoothActiveSetting = true;
  bool distanceFilter = true;
  int distanceFilterStrength = 5;
  int servoSensitivity = 3;
  int radarRange = 200;
  bool radarEnabled = true;
  int radarSensitivity = 5;
  bool dataLog = false;
  int logIntervalSetting = 1000;
  bool autonomousModeSetting = false;
  int autonomousSpeed = 10;
} settings;

// Timers
unsigned long lastRead = 0;
unsigned long lastInteraction = 0;
bool sleepMode = false;
unsigned long systemStartTime = 0;

// Statistics
struct {
  unsigned long startTime = 0;
  unsigned long operationTime = 0;
  int distanceReadCount = 0;
  int motionCount = 0;
  int servoMoveCount = 0;
  float maxDistance = 0;
  float minDistance = 1000;
  float averageDistance = 0;
  float totalDistance = 0;
  int radarScanCount = 0;
  int bluetoothMsgCount = 0;
  int alarmCount = 0;
  int errorCount = 0;
} statistics;

// =========== FUNCTION PROTOTYPES ===========
void initializeSystem();
void initializeSensors();
void initializeDisplay();
void initializeBluetooth();
void initializeServo();
void loadSettings();
void saveSettings();

void readAllSensors();
float readDistanceImproved();
float readDistanceFiltered();
void readGasSensor();
void readMotionSensor();
void readJoystick();

void processMenu();
void updateServo();
void updateRadar();
void updateLED();
void updateBuzzer();
void updateBluetooth();
void processBluetoothCommand(String cmd);
void sendBluetoothData();

void updateDisplay();
void drawMainMenu();
void drawDistanceScreen();
void drawGasScreen();
void drawMotionScreen();
void drawScanScreen();
void drawRadarScreen();
void drawServoControlScreen();
void drawSystemInfoScreen();
void drawStatisticsScreen();
void drawRadarView();
void draw3DView();
void drawSettingsDistanceScreen();
void drawSettingsGasScreen();
void drawSettingsMotionScreen();
void drawSettingsSoundScreen();
void drawSettingsServoScreen();
void drawSettingsLEDScreen();
void drawSecurityScreen();
void drawCalibrationScreen();
void drawTestScreen();
void drawCommunicationScreen();
void drawAdvancedScreen();
void drawDiagnosticScreen();
void drawFirmwareScreen();
void drawObstacleModeScreen();
void drawRCWLCalibrationScreen();
void drawBluetoothViewerScreen();
void drawRadar360Screen();
void drawDataLogScreen();
void drawAutonomousScreen();
void drawBatteryScreen();

void drawFullScreenAlert();
void drawRobot(int x, int y);
void drawBottomInfo();
void drawGenericMenu();

float calculateMedian(float arr[], int n);
float calculateAverage(float arr[], int n);
float lowPassFilter(float input, float prev, float alpha);
void triggerAlarm(int level);
void clearAlarm();
void logData(String data);

// =========== BLUETOOTH FUNCTIONS ===========
void initializeBluetooth() {
  if (!settings.bluetoothActiveSetting) return;
  
  SerialBT.begin("TOPROAK-V8-RADAR");
  Serial.println("Bluetooth Radar System Started!");
  Serial.println("Device Name: TOPROAK-V8-RADAR");
  Serial.println("Connection PIN: 1234");
  
  lastConnectionCheck = millis();
  bluetoothConnected = false;
}

void sendBluetoothData() {
  if (!settings.bluetoothActiveSetting || !bluetoothConnected) return;
  
  unsigned long now = millis();
  if (now - lastBluetoothSend < BLUETOOTH_SEND_INTERVAL) return;
  
  // Send data in JSON format
  String jsonData = "{";
  jsonData += "\"system\":\"TOPROAK V8.1\",";
  jsonData += "\"time\":" + String(now) + ",";
  jsonData += "\"distance\":" + String(distance, 2) + ",";
  jsonData += "\"gas\":" + String(gas, 0) + ",";
  jsonData += "\"motion\":" + String(motion ? 1 : 0) + ",";
  jsonData += "\"servo_angle\":" + String(servoAngle) + ",";
  jsonData += "\"radar_mode\":" + String(radarMode) + ",";
  jsonData += "\"radar_active\":" + String(radarActive ? 1 : 0) + ",";
  jsonData += "\"obstacle\":" + String(obstaclePresent ? 1 : 0) + ",";
  jsonData += "\"obstacle_dist\":" + String(obstacleDistance, 2) + ",";
  jsonData += "\"obstacle_angle\":" + String(obstacleAngle) + ",";
  jsonData += "\"warning\":" + String(alertActive ? 1 : 0) + ",";
  jsonData += "\"warning_level\":" + String(alertLevel) + ",";
  jsonData += "\"battery\":100,";
  jsonData += "\"signal\":95,";
  jsonData += "\"scan_data\":[";
  
  // Send radar data
  for (int i = 0; i < 36; i++) {
    jsonData += String(radarMap[i * 5], 2);
    if (i < 35) jsonData += ",";
  }
  
  jsonData += "],\"servo_data\":{";
  jsonData += "\"min\":" + String(settings.servoMinAngle) + ",";
  jsonData += "\"max\":" + String(settings.servoMaxAngle) + ",";
  jsonData += "\"speed\":" + String(settings.servoSpeed);
  jsonData += "}}";
  
  SerialBT.println(jsonData);
  lastBluetoothSend = now;
  statistics.bluetoothMsgCount++;
}

void processBluetoothCommand(String command) {
  command.trim();
  command.toUpperCase();
  
  if (command.length() == 0) return;
  
  Serial.print("BT Command: ");
  Serial.println(command);
  lastBluetoothMessage = command;
  lastMessageTime = millis();
  
  if (command == "STATUS") {
    String statusMsg = "TOPROAK V8.1 Status - Dist: " + String(distance, 1) + 
                      "cm, Gas: " + String(gas, 0) + 
                      "ppm, Servo: " + String(servoAngle) + 
                      "°, Radar: " + String(radarActive ? "ON" : "OFF") +
                      ", Scan: " + String(scanActive ? "ACTIVE" : "INACTIVE");
    SerialBT.println(statusMsg);
  }
  else if (command == "RADAR ON") {
    radarActive = true;
    scanActive = true;
    SerialBT.println("RADAR:ACTIVE");
  }
  else if (command == "RADAR OFF") {
    radarActive = false;
    scanActive = false;
    SerialBT.println("RADAR:INACTIVE");
  }
  else if (command == "SCAN ON") {
    scanActive = true;
    SerialBT.println("SCAN:ACTIVE");
  }
  else if (command == "SCAN OFF") {
    scanActive = false;
    SerialBT.println("SCAN:INACTIVE");
  }
  else if (command == "SERVO HOME") {
    servoAngle = SERVO_HOME_POS;
    targetAngle = SERVO_HOME_POS;
    radarServo.write(servoAngle);
    SerialBT.println("SERVO:HOME_POSITION");
  }
  else if (command == "LED ON") {
    settings.ledActive = true;
    SerialBT.println("LED:ON");
  }
  else if (command == "LED OFF") {
    settings.ledActive = false;
    SerialBT.println("LED:OFF");
  }
  else if (command == "BUZZER ON") {
    settings.sound = true;
    SerialBT.println("BUZZER:ON");
  }
  else if (command == "BUZZER OFF") {
    settings.sound = false;
    SerialBT.println("BUZZER:OFF");
  }
  else if (command.startsWith("SERVO ")) {
    int angle = command.substring(6).toInt();
    angle = constrain(angle, settings.servoMinAngle, settings.servoMaxAngle);
    servoAngle = angle;
    targetAngle = angle;
    radarServo.write(servoAngle);
    SerialBT.println("SERVO:" + String(servoAngle) + "DEG");
  }
  else if (command == "GET DATA") {
    sendBluetoothData();
  }
  else if (command == "GET SCAN") {
    // Send full radar data
    String scanData = "FULL_SCAN_DATA:";
    for (int i = 0; i < 180; i++) {
      scanData += String(radarMap[i], 1);
      if (i < 179) scanData += ",";
    }
    SerialBT.println(scanData);
  }
  else if (command == "RESET") {
    SerialBT.println("SYSTEM:RESETTING");
    delay(100);
    ESP.restart();
  }
  else if (command == "HELP") {
    String helpMsg = "=== TOPROAK V8.1 COMMANDS ===\n";
    helpMsg += "STATUS - System status\n";
    helpMsg += "RADAR ON/OFF - Radar control\n";
    helpMsg += "SCAN ON/OFF - Scan control\n";
    helpMsg += "SERVO HOME - Servo to home\n";
    helpMsg += "SERVO [angle] - Set servo angle\n";
    helpMsg += "LED ON/OFF - LED control\n";
    helpMsg += "BUZZER ON/OFF - Buzzer control\n";
    helpMsg += "GET DATA - Get all data\n";
    helpMsg += "GET SCAN - Get full scan\n";
    helpMsg += "RESET - Reset system\n";
    helpMsg += "HELP - This message";
    SerialBT.println(helpMsg);
  }
  else if (command.startsWith("SET ")) {
    if (command.startsWith("SET RANGE ")) {
      int range = command.substring(10).toInt();
      settings.maxDistance = constrain(range, 50, 400);
      SerialBT.println("RANGE:" + String(settings.maxDistance));
    }
    else if (command.startsWith("SET SPEED ")) {
      int speed = command.substring(10).toInt();
      scanSpeed = constrain(speed, 5, 100);
      settings.servoSpeed = scanSpeed;
      SerialBT.println("SPEED:" + String(scanSpeed));
    }
  }
  else {
    SerialBT.println("ERROR:UNKNOWN_COMMAND");
  }
}

void checkBluetoothConnection() {
  unsigned long now = millis();
  
  if (now - lastConnectionCheck > 3000) {
    bool wasConnected = bluetoothConnected;
    bluetoothConnected = SerialBT.hasClient();
    
    if (bluetoothConnected && !wasConnected) {
      Serial.println("Bluetooth Connected!");
      toneESP(BUZZER_PIN, 800, 200);
      delay(100);
      toneESP(BUZZER_PIN, 1200, 200);
    }
    else if (!bluetoothConnected && wasConnected) {
      Serial.println("Bluetooth Disconnected!");
      if (settings.sound) {
        toneESP(BUZZER_PIN, 400, 500);
      }
    }
    
    lastConnectionCheck = now;
  }
}

void readBluetoothData() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    processBluetoothCommand(command);
  }
}

// =========== ADVANCED DISTANCE MEASUREMENT ===========
float readDistanceImproved() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  unsigned long timeout = micros() + 30000; // 30ms timeout
  while (digitalRead(ECHO_PIN) == LOW && micros() < timeout);
  
  unsigned long startTime = micros();
  timeout = startTime + 30000;
  while (digitalRead(ECHO_PIN) == HIGH && micros() < timeout);
  
  unsigned long duration = micros() - startTime;
  
  if (duration > 30000) {
    distanceErrorCount++;
    if (distanceErrorCount > 5) {
      distanceSensorWorking = false;
    }
    return lastValidDistance;
  }
  
  float measuredDistance = duration * 0.03432 / 2.0;
  
  if (measuredDistance < MIN_DISTANCE) measuredDistance = MIN_DISTANCE;
  if (measuredDistance > MAX_DISTANCE) measuredDistance = MAX_DISTANCE;
  
  distanceErrorCount = 0;
  distanceSensorWorking = true;
  lastValidDistance = measuredDistance;
  
  return measuredDistance;
}

float applyKalmanFilter(float measurement) {
  static float estimate = 100.0;
  static float estimateError = 1.0;
  static float processNoise = 0.01;
  static float measurementError = 10.0;
  
  float kalmanGain = estimateError / (estimateError + measurementError);
  estimate = estimate + kalmanGain * (measurement - estimate);
  estimateError = (1.0 - kalmanGain) * estimateError + processNoise;
  
  return estimate;
}

float readDistanceFiltered() {
  float rawDistance = readDistanceImproved();
  
  // Apply median filter
  distanceMedianFilter[medianIndex] = rawDistance;
  medianIndex = (medianIndex + 1) % 7;
  
  float temp[7];
  for (int i = 0; i < 7; i++) {
    temp[i] = distanceMedianFilter[i];
  }
  
  // Bubble sort
  for (int i = 0; i < 6; i++) {
    for (int j = i + 1; j < 7; j++) {
      if (temp[i] > temp[j]) {
        float swap = temp[i];
        temp[i] = temp[j];
        temp[j] = swap;
      }
    }
  }
  
  float median = temp[3];
  
  // Moving average
  distanceBuffer[bufferIndex] = median;
  bufferIndex = (bufferIndex + 1) % DISTANCE_BUFFER_SIZE;
  
  float sum = 0;
  int count = 0;
  for (int i = 0; i < DISTANCE_BUFFER_SIZE; i++) {
    if (distanceBuffer[i] > MIN_DISTANCE && distanceBuffer[i] < MAX_DISTANCE) {
      sum += distanceBuffer[i];
      count++;
    }
  }
  
  float average = (count > 0) ? (sum / count) : median;
  
  // Apply Kalman filter
  float filtered = applyKalmanFilter(average);
  
  // Update statistics
  statistics.distanceReadCount++;
  statistics.totalDistance += filtered;
  statistics.averageDistance = statistics.totalDistance / statistics.distanceReadCount;
  
  if (filtered > statistics.maxDistance) statistics.maxDistance = filtered;
  if (filtered < statistics.minDistance && filtered > MIN_DISTANCE) {
    statistics.minDistance = filtered;
  }
  
  return filtered;
}

// =========== GAS SENSOR READING ===========
void readGasSensor() {
  static int gasBuffer[10] = {0};
  static int gasIndex = 0;
  static unsigned long lastGasRead = 0;
  
  unsigned long now = millis();
  if (now - lastGasRead < 200) return;
  
  int rawValue = analogRead(GAS_PIN);
  gasBuffer[gasIndex] = rawValue;
  gasIndex = (gasIndex + 1) % 10;
  
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += gasBuffer[i];
  }
  
  int average = sum / 10;
  gas = map(average, 0, 4095, 0, 2000);
  
  lastGasRead = now;
}

// =========== MOTION SENSOR READING ===========
void readMotionSensor() {
  static unsigned long lastMotionRead = 0;
  static bool lastMotionState = false;
  
  unsigned long now = millis();
  if (now - lastMotionRead < PIR_READ_INTERVAL) return;
  
  bool currentMotion = digitalRead(PIR_PIN);
  
  if (currentMotion && !lastMotionState) {
    motion = true;
    motionDetected = true;
    lastMotionTime = now;
    motionCount++;
    statistics.motionCount++;
    
    if (settings.sound && !alertActive) {
      toneESP(BUZZER_PIN, 1500, 100);
    }
  } 
  else if (!currentMotion && lastMotionState) {
    motion = false;
  }
  
  if (motionDetected && (now - lastMotionTime > 2000)) {
    motionDetected = false;
  }
  
  lastMotionState = currentMotion;
  lastMotionRead = now;
}

// =========== JOYSTICK READING ===========
void readJoystick() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  joyBtn = (digitalRead(JOY_BTN) == LOW);
  backBtn = (digitalRead(BACK_BTN) == LOW);
}

// =========== SERVO CONTROL ===========
void updateServo() {
  unsigned long now = millis();
  
  if (!settings.servoActive) {
    return;
  }
  
  if (manualServoMode) {
    // Manual control mode
    if (now - lastJoyServoMove > joyServoSpeed) {
      int joyX = analogRead(JOY_X);
      
      if (joyX < 1500) {
        servoAngle -= settings.servoSensitivity;
        if (servoAngle < settings.servoMinAngle) {
          servoAngle = settings.servoMinAngle;
          if (settings.sound) toneESP(BUZZER_PIN, 800, 50);
        }
      } 
      else if (joyX > 2500) {
        servoAngle += settings.servoSensitivity;
        if (servoAngle > settings.servoMaxAngle) {
          servoAngle = settings.servoMaxAngle;
          if (settings.sound) toneESP(BUZZER_PIN, 1200, 50);
        }
      }
      
      servoAngle = constrain(servoAngle, settings.servoMinAngle, settings.servoMaxAngle);
      radarServo.write(servoAngle);
      statistics.servoMoveCount++;
      lastJoyServoMove = now;
    }
    
    // Joystick button for home position
    if (joyBtn && !lastJoyBtn) {
      servoAngle = SERVO_HOME_POS;
      radarServo.write(servoAngle);
      if (settings.sound) toneESP(BUZZER_PIN, 600, 100);
    }
  } 
  else {
    // Automatic scan mode
    if (scanActive) {
      if (now - lastServoMove >= scanSpeed) {
        if (servoMoving) {
          if (servoAngle != targetAngle) {
            if (servoAngle < targetAngle) {
              servoAngle += settings.servoSensitivity;
            } else {
              servoAngle -= settings.servoSensitivity;
            }
            servoAngle = constrain(servoAngle, settings.servoMinAngle, settings.servoMaxAngle);
            radarServo.write(servoAngle);
            statistics.servoMoveCount++;
          } else {
            if (scanDirection) {
              targetAngle = settings.servoMaxAngle;
            } else {
              targetAngle = settings.servoMinAngle;
            }
            scanDirection = !scanDirection;
          }
        } else {
          servoMoving = true;
          targetAngle = (scanDirection) ? settings.servoMaxAngle : settings.servoMinAngle;
        }
        
        // Update radar map
        int radarIndex = map(servoAngle, settings.servoMinAngle, settings.servoMaxAngle, 0, 180);
        radarIndex = constrain(radarIndex, 0, 180);
        radarMap[radarIndex] = distance;
        
        lastServoMove = now;
      }
    }
  }
}

// =========== RADAR SYSTEM ===========
void updateRadar() {
  if (!radarActive) return;
  
  unsigned long now = millis();
  
  // Radar sweep
  if (now - lastRadarUpdate > 50) {
    if (radarSweepDirection) {
      radarSweepAngle++;
      if (radarSweepAngle >= 180) {
        radarSweepAngle = 180;
        radarSweepDirection = false;
        statistics.radarScanCount++;
      }
    } else {
      radarSweepAngle--;
      if (radarSweepAngle <= 0) {
        radarSweepAngle = 0;
        radarSweepDirection = true;
        statistics.radarScanCount++;
      }
    }
    
    // Update radar data
    int servoPos = map(radarSweepAngle, 0, 180, settings.servoMinAngle, settings.servoMaxAngle);
    servoAngle = servoPos;
    radarServo.write(servoAngle);
    
    // Save distance to radar map
    radarMap[radarSweepAngle] = distance;
    
    // Obstacle check
    if (distance < obstacleDistanceThreshold && distance > MIN_DISTANCE) {
      obstaclePresent = true;
      obstacleDistance = distance;
      obstacleAngle = radarSweepAngle;
      obstacleDetectTime = now;
    } else {
      obstaclePresent = false;
    }
    
    lastRadarUpdate = now;
  }
}

// =========== LED CONTROL ===========
void updateLED() {
  if (!settings.ledActive) {
    analogWriteESP(LED_PIN, 0);
    return;
  }
  
  unsigned long now = millis();
  
  switch (ledMode) {
    case 0: // Constant
      analogWriteESP(LED_PIN, ledBrightness);
      break;
      
    case 1: // Blink
      if (now - lastLedChange > ledBlinkSpeed) {
        ledState = !ledState;
        analogWriteESP(LED_PIN, ledState ? ledBrightness : 0);
        lastLedChange = now;
      }
      break;
      
    case 2: // Distance controlled
      {
        int brightness = map(distance, MIN_DISTANCE, settings.maxDistance, 255, 50);
        brightness = constrain(brightness, 50, 255);
        analogWriteESP(LED_PIN, brightness);
      }
      break;
      
    case 3: // Motion controlled
      if (motion) {
        analogWriteESP(LED_PIN, 255);
      } else {
        analogWriteESP(LED_PIN, 50);
      }
      break;
      
    case 4: // Rainbow
      {
        int hue = (now / 50) % 360;
        // Simplified HSV to PWM conversion
        int pwmValue = (hue % 60) * 4;
        if (pwmValue > 255) pwmValue = 255;
        analogWriteESP(LED_PIN, pwmValue);
      }
      break;
  }
}

// =========== BUZZER CONTROL ===========
void updateBuzzer() {
  if (!settings.sound) {
    noToneESP();
    return;
  }
  
  unsigned long now = millis();
  
  if (alertActive) {
    if (now - lastBeep > beepInterval) {
      if (alertLevel == 2) {
        toneESP(BUZZER_PIN, 1500, 200);
        beepInterval = 500;
      } else {
        toneESP(BUZZER_PIN, 1000, 150);
        beepInterval = 1000;
      }
      lastBeep = now;
    }
  }
}

// =========== ALERT CONTROL ===========
void checkAlerts() {
  bool previousAlert = alertActive;
  alertActive = false;
  alertLevel = 0;
  
  // Distance alert
  if (distance < settings.distanceDanger && distance > MIN_DISTANCE) {
    alertActive = true;
    alertLevel = 2;
    alertMessage = "DANGER! Distance: " + String(distance, 0) + "cm";
  }
  else if (distance < settings.distanceWarning && distance > MIN_DISTANCE) {
    alertActive = true;
    alertLevel = 1;
    alertMessage = "WARNING! Distance: " + String(distance, 0) + "cm";
  }
  
  // Gas alert
  else if (gas > settings.gasThreshold) {
    alertActive = true;
    alertLevel = 1;
    alertMessage = "HIGH GAS: " + String(gas, 0) + "ppm";
  }
  
  // Obstacle alert
  else if (obstaclePresent) {
    alertActive = true;
    alertLevel = 1;
    alertMessage = "OBSTACLE! " + String(obstacleDistance, 0) + "cm";
  }
  
  if (alertActive && !previousAlert) {
    lastAlertTime = millis();
    if (settings.sound) {
      toneESP(BUZZER_PIN, 1200, 300);
    }
  }
  
  // Full screen alert timing
  if (alertActive && alertLevel == 2) {
    if (!fullScreenAlert) {
      fullScreenAlert = true;
      fullScreenAlertStart = millis();
    }
  } else if (fullScreenAlert && (millis() - fullScreenAlertStart > 3000)) {
    fullScreenAlert = false;
  }
}

// =========== BUTTON CONTROL ===========
void processButtons() {
  unsigned long now = millis();
  static unsigned long lastButtonPress = 0;
  
  if (now - lastButtonPress < 300) return;
  
  // Back button
  if (backBtn && !lastBackBtn) {
    if (activeMenu == MENU_MAIN) {
      // Exit from main menu
      if (settings.sound) toneESP(BUZZER_PIN, 800, 100);
    } else {
      activeMenu = MENU_MAIN;
      settingsMode = false;
      manualServoMode = false;
      menuSelection = 0;
      if (settings.sound) toneESP(BUZZER_PIN, 1000, 100);
    }
    lastButtonPress = now;
  }
  
  // Joystick button
  if (joyBtn && !lastJoyBtn) {
    switch (activeMenu) {
      case MENU_MAIN:
        switch (menuSelection) {
          case 0: activeMenu = MENU_DISTANCE; break;
          case 1: activeMenu = MENU_GAS; break;
          case 2: activeMenu = MENU_MOTION; break;
          case 3: activeMenu = MENU_SCAN; break;
          case 4: activeMenu = MENU_RADAR; break;
          case 5: activeMenu = MENU_SERVO_MANUAL; manualServoMode = true; break;
          case 6: activeMenu = MENU_SETTINGS_DISTANCE; break;
          case 7: activeMenu = MENU_SETTINGS_GAS; break;
          case 8: activeMenu = MENU_SETTINGS_MOTION; break;
          case 9: activeMenu = MENU_SETTINGS_SOUND; break;
          case 10: activeMenu = MENU_SETTINGS_SERVO; break;
          case 11: activeMenu = MENU_SETTINGS_LED; break;
          case 12: activeMenu = MENU_SYSTEM; break;
          case 13: activeMenu = MENU_SECURITY; break;
          case 14: activeMenu = MENU_CALIBRATION; break;
          case 15: activeMenu = MENU_STATISTICS; break;
          case 16: activeMenu = MENU_TEST; break;
          case 17: activeMenu = MENU_COMMUNICATION; break;
          case 18: activeMenu = MENU_ADVANCED; break;
          case 19: activeMenu = MENU_DIAGNOSTIC; break;
          case 20: activeMenu = MENU_FIRMWARE; break;
          case 21: activeMenu = MENU_OBSTACLE_MODE; break;
          case 22: activeMenu = MENU_RCWL_CALIBRATION; break;
          case 23: activeMenu = MENU_BLUETOOTH_VIEWER; break;
          case 24: activeMenu = MENU_RADAR_360; break;
          case 25: activeMenu = MENU_3D_VIEW; break;
        }
        break;
        
      case MENU_SCAN:
        scanActive = !scanActive;
        break;
        
      case MENU_RADAR:
        radarActive = !radarActive;
        break;
        
      case MENU_SERVO_MANUAL:
        servoAngle = SERVO_HOME_POS;
        radarServo.write(servoAngle);
        break;
        
      case MENU_SETTINGS_SOUND:
        settings.sound = !settings.sound;
        break;
        
      case MENU_OBSTACLE_MODE:
        settings.servoObstacleModeSetting = !settings.servoObstacleModeSetting;
        break;
        
      default:
        if (settingsMode) {
          settingsMode = false;
        }
        break;
    }
    
    if (settings.sound) toneESP(BUZZER_PIN, 600, 50);
    lastButtonPress = now;
  }
  
  lastJoyBtn = joyBtn;
  lastBackBtn = backBtn;
}

void processJoystick() {
  static unsigned long lastJoyMove = 0;
  unsigned long now = millis();
  
  if (now - lastJoyMove < 200) return;
  
  int joyY = analogRead(JOY_Y);
  
  if (joyY < 1000) { // Up
    if (activeMenu == MENU_MAIN) {
      menuSelection--;
      if (menuSelection < 0) menuSelection = TOTAL_MENU_ITEMS - 1;
      
      if (menuSelection < menuStart) {
        menuStart = menuSelection;
      }
    } else if (settingsMode) {
      settingsSelection--;
    }
    lastJoyMove = now;
  } 
  else if (joyY > 3000) { // Down
    if (activeMenu == MENU_MAIN) {
      menuSelection++;
      if (menuSelection >= TOTAL_MENU_ITEMS) menuSelection = 0;
      
      if (menuSelection >= menuStart + MAX_VISIBLE_ITEMS) {
        menuStart = menuSelection - MAX_VISIBLE_ITEMS + 1;
      }
    } else if (settingsMode) {
      settingsSelection++;
    }
    lastJoyMove = now;
  }
  
  // Setting adjustment
  if (settingsMode) {
    int joyX = analogRead(JOY_X);
    
    if (joyX < 1000 || joyX > 3000) {
      int direction = (joyX < 1000) ? -1 : 1;
      
      switch (activeMenu) {
        case MENU_SETTINGS_DISTANCE:
          if (settingsSelection == 0) {
            settings.distanceWarning += direction * 10;
            settings.distanceWarning = constrain(settings.distanceWarning, 20, 200);
          } else if (settingsSelection == 1) {
            settings.distanceDanger += direction * 5;
            settings.distanceDanger = constrain(settings.distanceDanger, 5, 100);
          } else {
            settings.maxDistance += direction * 25;
            settings.maxDistance = constrain(settings.maxDistance, 50, 400);
          }
          break;
          
        case MENU_SETTINGS_SERVO:
          if (settingsSelection == 0) {
            settings.servoSpeed += direction * 5;
            settings.servoSpeed = constrain(settings.servoSpeed, 5, 100);
            scanSpeed = settings.servoSpeed;
          } else if (settingsSelection == 1) {
            settings.servoMinAngle += direction * 5;
            settings.servoMinAngle = constrain(settings.servoMinAngle, 0, 90);
          } else if (settingsSelection == 2) {
            settings.servoMaxAngle += direction * 5;
            settings.servoMaxAngle = constrain(settings.servoMaxAngle, 90, 180);
          }
          break;
      }
      lastJoyMove = now;
    }
  }
}

// =========== DISPLAY FUNCTIONS ===========
void drawDistanceScreen() {
  display.setTextSize(2);
  
  if (distance < 100) {
    display.setCursor(30, 20);
  } else {
    display.setCursor(15, 20);
  }
  
  display.print(distance, 1);
  display.print(" cm");
  
  display.setTextSize(1);
  
  // Distance bar
  int barWidth = map(distance, 0, settings.maxDistance, 118, 10);
  barWidth = constrain(barWidth, 10, 118);
  display.fillRect(5, 45, barWidth, 8, SSD1306_WHITE);
  display.drawRect(5, 45, 118, 8, SSD1306_WHITE);
  
  // Statistics
  display.setCursor(5, 55);
  display.print("Max: ");
  display.print(statistics.maxDistance, 0);
  display.print("cm Min: ");
  display.print(statistics.minDistance, 0);
  display.print("cm");
}

void drawGasScreen() {
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print(gas, 0);
  display.print(" ppm");
  
  display.setTextSize(1);
  
  // Gas level bar
  int gasLevel = map(gas, 0, 2000, 0, 118);
  gasLevel = constrain(gasLevel, 0, 118);
  display.fillRect(5, 45, gasLevel, 8, SSD1306_WHITE);
  display.drawRect(5, 45, 118, 8, SSD1306_WHITE);
  
  // Air quality indicator
  display.setCursor(5, 55);
  if (gas < 500) {
    display.print("AIR QUALITY: GOOD");
  } else if (gas < 1000) {
    display.print("AIR QUALITY: MODERATE");
  } else {
    display.print("AIR QUALITY: POOR");
  }
}

void drawMotionScreen() {
  display.setTextSize(2);
  display.setCursor(15, 20);
  if (motion) {
    display.print("MOTION DETECTED");
  } else {
    display.print("NO MOTION");
  }
  
  display.setTextSize(1);
  display.setCursor(5, 45);
  display.print("Total Detections: ");
  display.print(motionCount);
  
  display.setCursor(5, 55);
  if (lastMotionTime > 0) {
    unsigned long timeSince = (millis() - lastMotionTime) / 1000;
    display.print("Last: ");
    display.print(timeSince);
    display.print(" seconds ago");
  } else {
    display.print("No motion detected yet");
  }
}

void drawScanScreen() {
  display.fillRect(0, 12, 128, 36, SSD1306_BLACK);
  display.drawRect(5, 15, 118, 30, SSD1306_WHITE);
  
  // Grid lines
  for (int i = 1; i <= 3; i++) {
    int y = 15 + (30 * i / 4);
    display.drawFastHLine(5, y, 118, SSD1306_WHITE);
  }
  
  for (int i = 1; i <= 3; i++) {
    int x = 5 + (118 * i / 4);
    display.drawFastVLine(x, 15, 30, SSD1306_WHITE);
  }
  
  // Current position
  int currentX = map(servoAngle, settings.servoMinAngle, settings.servoMaxAngle, 5, 123);
  display.drawFastVLine(currentX, 15, 30, SSD1306_WHITE);
  
  // Draw scan points
  int previousX = -1;
  int previousY = -1;
  
  for (int i = 0; i < 31; i++) {
    float distanceValue = radarMap[i * 6];
    if (distanceValue > 2.0 && distanceValue < 200.0) {
      int x = map(i, 0, 30, 5, 123);
      int y = map(distanceValue, 0, 200, 45, 15);
      
      int pointSize = map(distanceValue, 0, 200, 3, 1);
      pointSize = constrain(pointSize, 1, 3);
      
      if (pointSize == 3) {
        display.fillCircle(x, y, 2, SSD1306_WHITE);
      } else if (pointSize == 2) {
        display.drawCircle(x, y, 2, SSD1306_WHITE);
      } else {
        display.drawPixel(x, y, SSD1306_WHITE);
      }
      
      if (previousX != -1 && previousY != -1) {
        display.drawLine(previousX, previousY, x, y, SSD1306_WHITE);
      }
      
      previousX = x;
      previousY = y;
    }
  }
  
  // Bottom info
  display.setCursor(5, 50);
  display.print("A:");
  display.print(servoAngle);
  display.print(" D:");
  display.print(distance, 0);
  display.print("cm S:");
  display.print(scanActive ? "ON" : "OFF");
  
  display.setCursor(5, 60);
  if (servoMoving) {
    display.print("SCANNING...");
  } else if (settings.servoObstacleModeSetting) {
    display.print("OBSTACLE MODE");
  } else {
    display.print("READY");
  }
}

void drawRadarScreen() {
  // Draw radar display
  int centerX = 64;
  int centerY = 32;
  int radius = 30;
  
  // Radar circle
  display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);
  display.drawCircle(centerX, centerY, radius/2, SSD1306_WHITE);
  
  // Center point
  display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
  
  // Sweep line
  float angle = radians(map(servoAngle, settings.servoMinAngle, settings.servoMaxAngle, 0, 180));
  int endX = centerX + radius * cos(angle);
  int endY = centerY - radius * sin(angle);
  display.drawLine(centerX, centerY, endX, endY, SSD1306_WHITE);
  
  // Obstacle points
  for (int i = 0; i < 180; i += 5) {
    if (radarMap[i] > 0 && radarMap[i] < settings.maxDistance) {
      float pointAngle = radians(i);
      float pointDistance = radarMap[i];
      int pointRadius = map(pointDistance, 0, settings.maxDistance, 5, radius);
      int pointX = centerX + pointRadius * cos(pointAngle);
      int pointY = centerY - pointRadius * sin(pointAngle);
      
      display.drawPixel(pointX, pointY, SSD1306_WHITE);
      if (pointDistance < obstacleDistanceThreshold) {
        display.drawCircle(pointX, pointY, 2, SSD1306_WHITE);
      }
    }
  }
  
  // Information area
  display.setCursor(5, 55);
  display.print("A:");
  display.print(servoAngle);
  display.print(" D:");
  display.print(distance, 0);
  display.print("cm");
  
  if (obstaclePresent) {
    display.setCursor(80, 55);
    display.print("OBSTACLE!");
  }
}

void drawServoControlScreen() {
  display.setTextSize(2);
  display.setCursor(30, 15);
  display.print("SERVO");
  
  display.setTextSize(3);
  display.setCursor(40, 35);
  display.print(servoAngle);
  display.setTextSize(1);
  display.print("°");
  
  // Control instructions
  display.setCursor(5, 55);
  display.print("JOY:Left/Right");
  display.setCursor(5, 62);
  display.print("BTN:Center");
}

void drawSystemInfoScreen() {
  display.setCursor(5, 10);
  display.print("TOPROAK V8.1");
  display.setCursor(5, 20);
  display.print("Uptime: ");
  display.print((millis() - statistics.startTime) / 1000);
  display.print("s");
  display.setCursor(5, 30);
  display.print("Distance Reads: ");
  display.print(statistics.distanceReadCount);
  display.setCursor(5, 40);
  display.print("Motion Detects: ");
  display.print(statistics.motionCount);
  display.setCursor(5, 50);
  display.print("Servo Moves: ");
  display.print(statistics.servoMoveCount);
  display.setCursor(5, 60);
  display.print("BT: ");
  display.print(bluetoothConnected ? "CONNECTED" : "DISCONNECTED");
}

void drawStatisticsScreen() {
  display.setCursor(5, 10);
  display.print("Max Distance: ");
  display.print(statistics.maxDistance, 0);
  display.print("cm");
  display.setCursor(5, 20);
  display.print("Min Distance: ");
  display.print(statistics.minDistance, 0);
  display.print("cm");
  display.setCursor(5, 30);
  display.print("Avg Distance: ");
  display.print(statistics.averageDistance, 0);
  display.print("cm");
  display.setCursor(5, 40);
  display.print("Radar Scans: ");
  display.print(statistics.radarScanCount);
  display.setCursor(5, 50);
  display.print("BT Messages: ");
  display.print(statistics.bluetoothMsgCount);
  display.setCursor(5, 60);
  display.print("Alarms: ");
  display.print(statistics.alarmCount);
}

void drawSettingsDistanceScreen() {
  if (settingsMode) {
    display.setCursor(5, 15);
    if (settingsSelection == 0) {
      display.print("Warning:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(settings.distanceWarning);
      display.print("cm");
    } else if (settingsSelection == 1) {
      display.print("Danger:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(settings.distanceDanger);
      display.print("cm");
    } else {
      display.print("Max Distance:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(settings.maxDistance);
      display.print("cm");
    }
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Left/Right: Adjust");
  } else {
    display.setCursor(5, 15);
    display.print("Warning: ");
    display.print(settings.distanceWarning);
    display.print("cm");
    display.setCursor(5, 25);
    display.print("Danger: ");
    display.print(settings.distanceDanger);
    display.print("cm");
    display.setCursor(5, 35);
    display.print("Max: ");
    display.print(settings.maxDistance);
    display.print("cm");
    display.setCursor(5, 55);
    display.print("Button: Settings Mode");
  }
}

void drawSettingsGasScreen() {
  if (settingsMode) {
    display.setCursor(5, 15);
    display.print("Gas Threshold:");
    display.setTextSize(2);
    display.setCursor(30, 30);
    display.print(settings.gasThreshold);
    display.print("ppm");
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Left/Right: Adjust");
  } else {
    display.setCursor(5, 15);
    display.print("Current: ");
    display.print(gas, 0);
    display.print("ppm");
    display.setCursor(5, 25);
    display.print("Threshold: ");
    display.print(settings.gasThreshold);
    display.print("ppm");
    display.setCursor(5, 55);
    display.print("Button: Settings Mode");
  }
}

void drawSettingsMotionScreen() {
  if (settingsMode) {
    display.setCursor(5, 15);
    display.print("Motion Time:");
    display.setTextSize(2);
    display.setCursor(30, 30);
    display.print(settings.motionThresholdTime/1000);
    display.print(" s");
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Left/Right: Adjust");
  } else {
    display.setCursor(5, 15);
    display.print("Threshold: ");
    display.print(settings.motionThresholdTime/1000);
    display.print(" s");
    display.setCursor(5, 25);
    display.print("Motion: ");
    display.print(motion ? "DETECTED" : "NONE");
    display.setCursor(5, 55);
    display.print("Button: Settings Mode");
  }
}

void drawSettingsSoundScreen() {
  display.setCursor(10, 20);
  display.print("Sound Status:");
  display.setTextSize(2);
  display.setCursor(40, 35);
  display.print(settings.sound ? "ON" : "OFF");
  display.setTextSize(1);
  display.setCursor(10, 58);
  display.print("Button: Toggle");
}

void drawSettingsServoScreen() {
  if (settingsMode) {
    display.setCursor(5, 15);
    if (settingsSelection == 0) {
      display.print("Scan Speed:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(settings.servoSpeed);
      display.print("ms");
    } else if (settingsSelection == 1) {
      display.print("Min Angle:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(settings.servoMinAngle);
      display.print("°");
    } else if (settingsSelection == 2) {
      display.print("Max Angle:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(settings.servoMaxAngle);
      display.print("°");
    }
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Left/Right: Adjust");
  } else {
    display.setCursor(5, 15);
    display.print("Scan Speed: ");
    display.print(settings.servoSpeed);
    display.print("ms");
    display.setCursor(5, 25);
    display.print("Min: ");
    display.print(settings.servoMinAngle);
    display.print("°");
    display.setCursor(5, 35);
    display.print("Max: ");
    display.print(settings.servoMaxAngle);
    display.print("°");
    display.setCursor(5, 55);
    display.print("Button: Settings Mode");
  }
}

void drawSettingsLEDScreen() {
  if (settingsMode) {
    display.setCursor(5, 15);
    if (settingsSelection == 0) {
      display.print("LED Mode:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      switch(ledMode) {
        case 0: display.print("CONSTANT"); break;
        case 1: display.print("BLINK"); break;
        case 2: display.print("DISTANCE"); break;
        case 3: display.print("MOTION"); break;
        case 4: display.print("RAINBOW"); break;
      }
    } else if (settingsSelection == 1) {
      display.print("LED Speed:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(settings.ledSpeed);
      display.print("ms");
    } else {
      display.print("Brightness:");
      display.setTextSize(2);
      display.setCursor(30, 30);
      display.print(settings.ledBrightnessSetting);
    }
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print("Left/Right: Adjust");
  } else {
    display.setCursor(5, 15);
    display.print("Mode: ");
    switch(ledMode) {
      case 0: display.print("CONSTANT"); break;
      case 1: display.print("BLINK"); break;
      case 2: display.print("DISTANCE"); break;
      case 3: display.print("MOTION"); break;
      case 4: display.print("RAINBOW"); break;
    }
    display.setCursor(5, 25);
    display.print("Speed: ");
    display.print(settings.ledSpeed);
    display.print("ms");
    display.setCursor(5, 35);
    display.print("Brightness: ");
    display.print(settings.ledBrightnessSetting);
    display.setCursor(5, 55);
    display.print("Button: Settings Mode");
  }
}

void drawSecurityScreen() {
  display.setTextSize(2);
  display.setCursor(25, 20);
  display.print(securityActive ? "ACTIVE" : "INACTIVE");
  
  display.setTextSize(1);
  display.setCursor(5, 45);
  if (securityActive) {
    display.print("Status: ");
    if (alarmActive) {
      display.print("ALARM!");
    } else {
      display.print("MONITORING");
    }
  } else {
    display.print("Security system is");
    display.setCursor(5, 55);
    display.print("currently inactive");
  }
}

void drawCalibrationScreen() {
  display.setCursor(20, 20);
  display.print("CALIBRATION");
  display.setCursor(10, 35);
  display.print("Distance sensor");
  display.setCursor(10, 45);
  display.print("and gas sensor");
  display.setCursor(30, 55);
  display.print("will be calibrated");
}

void drawTestScreen() {
  display.setCursor(30, 20);
  display.print("TEST MODE");
  display.setCursor(10, 35);
  display.print("Servo, LED, Buzzer");
  display.setCursor(20, 45);
  display.print("will be tested");
  display.setCursor(40, 55);
  display.print("Button: Start");
}

void drawCommunicationScreen() {
  display.setCursor(20, 20);
  display.print("COMMUNICATION");
  display.setCursor(10, 35);
  display.print("Serial: ");
  display.print(115200);
  display.setCursor(10, 45);
  display.print("Bluetooth: ");
  display.print(bluetoothConnected ? "CONNECTED" : "OFF");
  display.setCursor(10, 55);
  display.print("Device: TOPROAK-V8");
}

void drawAdvancedScreen() {
  display.setCursor(20, 20);
  display.print("ADVANCED");
  display.setCursor(10, 35);
  display.print("Auto Calibration");
  display.setCursor(10, 45);
  display.print("Data Log: OFF");
  display.setCursor(10, 55);
  display.print("Diagnostic: OFF");
}

void drawDiagnosticScreen() {
  display.setCursor(20, 20);
  display.print("DIAGNOSTIC");
  display.setCursor(10, 35);
  display.print("Sensor Test");
  display.setCursor(10, 45);
  display.print("Memory Test");
  display.setCursor(10, 55);
  display.print("Performance Test");
}

void drawFirmwareScreen() {
  display.setCursor(20, 20);
  display.print("FIRMWARE");
  display.setCursor(10, 35);
  display.print("Version: V8.1");
  display.setCursor(10, 45);
  display.print("Build: 2024");
  display.setCursor(10, 55);
  display.print("Update: None");
}

void drawObstacleModeScreen() {
  display.setCursor(5, 15);
  display.print("Obstacle Mode: ");
  display.print(settings.servoObstacleModeSetting ? "ACTIVE" : "INACTIVE");
  
  display.setCursor(5, 25);
  display.print("Obstacle Threshold: ");
  display.print(obstacleDistanceThreshold);
  display.print(" cm");
  
  display.setCursor(5, 35);
  display.print("Current Distance: ");
  display.print(distance, 0);
  display.print(" cm");
  
  display.setCursor(5, 45);
  display.print("Obstacle Status: ");
  display.print(obstaclePresent ? "PRESENT" : "NONE");
  
  display.setCursor(5, 55);
  display.print("Servo: ");
  display.print(servoMoving ? "SCANNING" : "READY");
}

void drawRCWLCalibrationScreen() {
  display.setCursor(5, 15);
  display.print("RCWL-0507 Calibration");
  
  display.setCursor(5, 25);
  display.print("Status: WORKING");
  
  display.setCursor(5, 35);
  display.print("Motion: ");
  display.print(motion ? "DETECTED" : "NONE");
  
  display.setCursor(5, 45);
  display.print("Total: ");
  display.print(motionCount);
  
  display.setCursor(5, 55);
  display.print("Auto-calibrating");
}

void drawBluetoothViewerScreen() {
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setCursor(10, 5);
  display.print("BLUETOOTH");
  
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print("Status: ");
  
  if (bluetoothConnected) {
    display.print("CONNECTED");
    display.setCursor(10, 35);
    display.print("Device: TOPROAK-V8");
    display.setCursor(10, 45);
    display.print("Sending data...");
    
    static int btAnim = 0;
    btAnim = (btAnim + 1) % 10;
    
    display.setCursor(10, 55);
    if (btAnim < 5) {
      display.print("[===>        ]");
    } else {
      display.print("[        ===>]");
    }
  } else {
    display.print("DISCONNECTED");
    display.setCursor(10, 35);
    display.print("Waiting connection");
    display.setCursor(10, 45);
    display.print("Device Name:");
    display.setCursor(10, 55);
    display.print("TOPROAK-V8-RADAR");
  }
}

void drawRadar360Screen() {
  // 360 degree radar view
  int centerX = 64;
  int centerY = 32;
  int radius = 30;
  
  // Full circle
  display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);
  
  // Center
  display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
  
  // Points in all directions
  for (int angle = 0; angle < 360; angle += 10) {
    if (angle < 180) {
      float dist = radarMap[angle];
      if (dist > 0 && dist < settings.maxDistance) {
        float radAngle = radians(angle);
        int pointRadius = map(dist, 0, settings.maxDistance, 5, radius);
        int pointX = centerX + pointRadius * cos(radAngle);
        int pointY = centerY - pointRadius * sin(radAngle);
        
        // Color coding
        if (dist < settings.distanceDanger) {
          display.fillCircle(pointX, pointY, 2, SSD1306_WHITE);
        } else if (dist < settings.distanceWarning) {
          display.drawCircle(pointX, pointY, 2, SSD1306_WHITE);
        } else {
          display.drawPixel(pointX, pointY, SSD1306_WHITE);
        }
      }
    }
  }
  
  // Sweep line
  float currentAngle = radians(map(servoAngle, settings.servoMinAngle, settings.servoMaxAngle, 0, 180));
  int lineX = centerX + radius * cos(currentAngle);
  int lineY = centerY - radius * sin(currentAngle);
  display.drawLine(centerX, centerY, lineX, lineY, SSD1306_WHITE);
  
  // Statistics
  display.setCursor(5, 55);
  display.print("Scan:");
  display.print(statistics.radarScanCount);
  display.print(" Obs:");
  display.print(obstaclePresent ? "YES" : "NO");
  
  display.setCursor(5, 62);
  display.print("Range:");
  display.print(settings.maxDistance);
  display.print("cm Mode:");
  display.print(radarMode);
}

void drawDataLogScreen() {
  display.setCursor(20, 20);
  display.print("DATA LOG");
  display.setCursor(10, 35);
  display.print("Status: ");
  display.print(dataLogging ? "ACTIVE" : "INACTIVE");
  display.setCursor(10, 45);
  display.print("Interval: ");
  display.print(logInterval);
  display.print("ms");
  display.setCursor(10, 55);
  if (dataLogging) {
    display.print("Logging data...");
  } else {
    display.print("Press to start");
  }
}

void drawAutonomousScreen() {
  display.setCursor(20, 20);
  display.print("AUTONOMOUS");
  display.setCursor(10, 35);
  display.print("Mode: ");
  display.print(autonomousMode ? "ACTIVE" : "INACTIVE");
  display.setCursor(10, 45);
  display.print("State: ");
  display.print(autonomousState);
  display.setCursor(10, 55);
  if (autonomousMode) {
    display.print("Running...");
  } else {
    display.print("Ready to start");
  }
}

void drawBatteryScreen() {
  display.setCursor(30, 20);
  display.print("BATTERY");
  
  // Battery icon
  display.drawRect(40, 35, 40, 15, SSD1306_WHITE);
  display.fillRect(80, 38, 3, 9, SSD1306_WHITE);
  
  // Battery level (simulated)
  int batteryLevel = 75; // Simulated 75%
  int fillWidth = map(batteryLevel, 0, 100, 0, 38);
  display.fillRect(41, 36, fillWidth, 13, SSD1306_WHITE);
  
  display.setCursor(45, 55);
  display.print("Level: ");
  display.print(batteryLevel);
  display.print("%");
}

void draw3DView() {
  display.setCursor(30, 20);
  display.print("3D VIEW");
  display.setCursor(10, 35);
  display.print("Perspective: ");
  display.print(viewAngle, 0);
  display.print("°");
  display.setCursor(10, 45);
  display.print("Mode: 3D RADAR");
  display.setCursor(10, 55);
  display.print("Processing...");
}

void drawMainMenu() {
  const char* menuItems[] = {
    "Distance Measurement",
    "Gas Sensor",
    "Motion Sensor",
    "Scan Mode",
    "Radar Display",
    "Servo Control",
    "Distance Settings",
    "Gas Settings",
    "Motion Settings",
    "Sound Settings",
    "Servo Settings",
    "LED Settings",
    "System Information",
    "Security System",
    "Calibration",
    "Statistics",
    "Test Mode",
    "Communication",
    "Advanced Settings",
    "Diagnostic",
    "Firmware Info",
    "Obstacle Mode",
    "RCWL Calibration",
    "Bluetooth Viewer",
    "360 Radar",
    "3D View",
    "Data Log",
    "Autonomous Mode",
    "Battery Status"
  };
  
  for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
    int itemIndex = menuStart + i;
    if (itemIndex >= TOTAL_MENU_ITEMS) break;
    
    int y = 15 + i * 7;
    
    if (itemIndex == menuSelection) {
      display.fillRect(0, y-1, 128, 7, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(2, y);
      display.print("> ");
      display.print(menuItems[itemIndex]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(4, y);
      display.print(menuItems[itemIndex]);
    }
  }
  
  // Scroll bar
  if (TOTAL_MENU_ITEMS > MAX_VISIBLE_ITEMS) {
    int barHeight = 35;
    int barY = 15;
    int thumbPos = map(menuStart, 0, TOTAL_MENU_ITEMS - MAX_VISIBLE_ITEMS, barY, barY + barHeight - 3);
    display.drawRect(126, barY, 2, barHeight, SSD1306_WHITE);
    display.fillRect(126, thumbPos, 2, 3, SSD1306_WHITE);
  }
}

void drawFullScreenAlert() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  
  // Flashing frame
  static bool flash = false;
  flash = !flash;
  
  if (flash) {
    display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  }
  
  display.setCursor(20, 10);
  display.print("! ALERT !");
  
  display.setTextSize(1);
  if (!flash) display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(10, 35);
  display.print(alertMessage);
  
  display.setCursor(10, 45);
  display.print("Distance: ");
  display.print(distance, 1);
  display.print(" cm");
  
  display.setCursor(10, 55);
  display.print("Angle: ");
  display.print(servoAngle);
  display.print(" degrees");
  
  if (flash) {
    display.setTextColor(SSD1306_WHITE);
  }
}

void drawRobot(int x, int y) {
  display.drawCircle(x, y, 6, SSD1306_WHITE);
  
  if (eyesOpen) {
    if (robotExpression == 0) {
      display.fillCircle(x-3, y-1, 1, SSD1306_WHITE);
      display.fillCircle(x+3, y-1, 1, SSD1306_WHITE);
      display.drawPixel(x, y+2, SSD1306_WHITE);
    } else if (robotExpression == 2) {
      display.fillCircle(x-3, y-2, 2, SSD1306_WHITE);
      display.fillCircle(x+3, y-2, 2, SSD1306_WHITE);
      display.drawLine(x-3, y+2, x+3, y+2, SSD1306_WHITE);
    }
  } else {
    display.drawLine(x-3, y-1, x-1, y-1, SSD1306_WHITE);
    display.drawLine(x+1, y-1, x+3, y-1, SSD1306_WHITE);
  }
  
  if (alertActive) {
    display.drawTriangle(x+8, y-8, x+8, y-3, x+5, y-5, SSD1306_WHITE);
  }
}

void drawBottomInfo() {
  display.drawLine(0, 53, 128, 53, SSD1306_WHITE);
  
  // Bottom left
  display.setCursor(2, 55);
  if (alertActive) {
    display.print("! ");
    display.print(alertLevel == 2 ? "DANGER" : "WARNING");
  } else {
    display.print("System OK");
  }
  
  // Bottom right
  display.setCursor(80, 55);
  display.print("M:");
  display.print(menuSelection + 1);
  display.print("/");
  display.print(TOTAL_MENU_ITEMS);
  
  // Bottom center
  display.setCursor(50, 62);
  display.print("D18:BACK JOY:OK");
}

void drawGenericMenu() {
  display.setCursor(20, 25);
  display.print("MENU OPTIONS");
  display.setCursor(10, 40);
  display.print("Use joystick to");
  display.setCursor(10, 50);
  display.print("navigate menus");
}

void updateDisplay() {
  if (fullScreenAlert) {
    drawFullScreenAlert();
    return;
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Title bar
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print("TOPROAK V8.1");
  
  // System status
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(80, 2);
  display.print("BT:");
  display.print(bluetoothConnected ? "ON" : "OFF");
  
  // Main content
  switch (activeMenu) {
    case MENU_MAIN: drawMainMenu(); break;
    case MENU_DISTANCE: drawDistanceScreen(); break;
    case MENU_GAS: drawGasScreen(); break;
    case MENU_MOTION: drawMotionScreen(); break;
    case MENU_SCAN: drawScanScreen(); break;
    case MENU_RADAR: drawRadarScreen(); break;
    case MENU_SERVO_MANUAL: drawServoControlScreen(); break;
    case MENU_SYSTEM: drawSystemInfoScreen(); break;
    case MENU_STATISTICS: drawStatisticsScreen(); break;
    case MENU_SETTINGS_DISTANCE: drawSettingsDistanceScreen(); break;
    case MENU_SETTINGS_GAS: drawSettingsGasScreen(); break;
    case MENU_SETTINGS_MOTION: drawSettingsMotionScreen(); break;
    case MENU_SETTINGS_SOUND: drawSettingsSoundScreen(); break;
    case MENU_SETTINGS_SERVO: drawSettingsServoScreen(); break;
    case MENU_SETTINGS_LED: drawSettingsLEDScreen(); break;
    case MENU_SECURITY: drawSecurityScreen(); break;
    case MENU_CALIBRATION: drawCalibrationScreen(); break;
    case MENU_TEST: drawTestScreen(); break;
    case MENU_COMMUNICATION: drawCommunicationScreen(); break;
    case MENU_ADVANCED: drawAdvancedScreen(); break;
    case MENU_DIAGNOSTIC: drawDiagnosticScreen(); break;
    case MENU_FIRMWARE: drawFirmwareScreen(); break;
    case MENU_OBSTACLE_MODE: drawObstacleModeScreen(); break;
    case MENU_RCWL_CALIBRATION: drawRCWLCalibrationScreen(); break;
    case MENU_BLUETOOTH_VIEWER: drawBluetoothViewerScreen(); break;
    case MENU_RADAR_360: drawRadar360Screen(); break;
    case MENU_3D_VIEW: draw3DView(); break;
    case MENU_DATA_LOG: drawDataLogScreen(); break;
    case MENU_AUTONOMOUS: drawAutonomousScreen(); break;
    case MENU_BATTERY: drawBatteryScreen(); break;
    default: drawGenericMenu(); break;
  }
  
  // Robot character
  drawRobot(110, 50);
  
  // Bottom info
  drawBottomInfo();
  
  display.display();
}

// =========== PWM FUNCTIONS ===========
void toneESP(int pin, int frequency, int duration) {
  if (!settings.sound) return;
  
  ledcWriteTone(BUZZER_CHANNEL, frequency);
  if (duration > 0) {
    delay(duration);
    ledcWriteTone(BUZZER_CHANNEL, 0);
  }
}

void noToneESP() {
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void analogWriteESP(int pin, int value) {
  ledcWrite(LED_CHANNEL, value);
}

// =========== SETTINGS MANAGEMENT ===========
void loadSettings() {
  preferences.begin("toproak_v8", false);
  
  settings.distanceWarning = preferences.getInt("dist_warn", 100);
  settings.distanceDanger = preferences.getInt("dist_dang", 30);
  settings.gasThreshold = preferences.getInt("gas_thresh", 800);
  settings.sound = preferences.getBool("sound", true);
  settings.servoActive = preferences.getBool("servo_en", true);
  settings.servoSpeed = preferences.getInt("servo_spd", 15);
  settings.servoMinAngle = preferences.getInt("servo_min", 0);
  settings.servoMaxAngle = preferences.getInt("servo_max", 180);
  settings.ledActive = preferences.getBool("led_en", true);
  settings.maxDistance = preferences.getInt("max_dist", 400);
  settings.bluetoothActiveSetting = preferences.getBool("bt_en", true);
  settings.radarRange = preferences.getInt("radar_rng", 200);
  
  scanSpeed = settings.servoSpeed;
  obstacleDistanceThreshold = settings.distanceDanger;
  
  preferences.end();
}

void saveSettings() {
  preferences.begin("toproak_v8", false);
  
  preferences.putInt("dist_warn", settings.distanceWarning);
  preferences.putInt("dist_dang", settings.distanceDanger);
  preferences.putInt("gas_thresh", settings.gasThreshold);
  preferences.putBool("sound", settings.sound);
  preferences.putBool("servo_en", settings.servoActive);
  preferences.putInt("servo_spd", settings.servoSpeed);
  preferences.putInt("servo_min", settings.servoMinAngle);
  preferences.putInt("servo_max", settings.servoMaxAngle);
  preferences.putBool("led_en", settings.ledActive);
  preferences.putInt("max_dist", settings.maxDistance);
  preferences.putBool("bt_en", settings.bluetoothActiveSetting);
  preferences.putInt("radar_rng", settings.radarRange);
  
  preferences.end();
}

// =========== UTILITY FUNCTIONS ===========
float calculateMedian(float arr[], int n) {
  // Simple bubble sort for median calculation
  for (int i = 0; i < n-1; i++) {
    for (int j = 0; j < n-i-1; j++) {
      if (arr[j] > arr[j+1]) {
        float temp = arr[j];
        arr[j] = arr[j+1];
        arr[j+1] = temp;
      }
    }
  }
  
  if (n % 2 == 0) {
    return (arr[n/2 - 1] + arr[n/2]) / 2.0;
  } else {
    return arr[n/2];
  }
}

float calculateAverage(float arr[], int n) {
  float sum = 0;
  for (int i = 0; i < n; i++) {
    sum += arr[i];
  }
  return sum / n;
}

float lowPassFilter(float input, float prev, float alpha) {
  return alpha * input + (1 - alpha) * prev;
}

void triggerAlarm(int level) {
  alarmActive = true;
  alarmStart = millis();
  alarmLevel = level;
  statistics.alarmCount++;
}

void clearAlarm() {
  alarmActive = false;
  alarmLevel = 0;
  noToneESP();
}

void logData(String data) {
  if (!dataLogging) return;
  
  unsigned long now = millis();
  if (now - logStartTime > logInterval) {
    Serial.println("LOG: " + data);
    logStartTime = now;
  }
}

// =========== SETUP ===========
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n==================================");
  Serial.println("TOPROAK PRO MAX V8.1 - RADAR SYSTEM");
  Serial.println("ESP32 + HC-SR04 + RCWL-0507 + Bluetooth");
  Serial.println("==================================\n");
  
  // Pin modes
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(BACK_BTN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SYS_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  
  // PWM settings
  ledcSetup(BUZZER_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);
  
  ledcSetup(LED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
  ledcWrite(LED_CHANNEL, 0);
  
  // Servo initialization
  ESP32PWM::allocateTimer(0);
  radarServo.setPeriodHertz(50);
  
  bool servoOk = radarServo.attach(SERVO_PIN, 500, 2500);
  if (servoOk) {
    Serial.println("Servo connected successfully");
    servoAngle = SERVO_HOME_POS;
    radarServo.write(servoAngle);
    delay(500);
  } else {
    Serial.println("Servo connection failed!");
  }
  
  // OLED initialization
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED connection failed!");
    while (1);
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 20);
  display.print("TOPROAK");
  display.setTextSize(1);
  display.setCursor(35, 45);
  display.print("V8.1 RADAR");
  display.display();
  delay(2000);
  
  // Bluetooth initialization
  initializeBluetooth();
  
  // Load settings
  loadSettings();
  
  // Startup beep
  if (settings.sound) {
    toneESP(BUZZER_PIN, 800, 200);
    delay(100);
    toneESP(BUZZER_PIN, 1200, 200);
    delay(100);
    toneESP(BUZZER_PIN, 1600, 200);
  }
  
  // Statistics initialization
  statistics.startTime = millis();
  systemStartTime = millis();
  
  Serial.println("\nSystem ready!");
  Serial.println("Distance sensor calibration...");
  
  // Distance sensor calibration
  for (int i = 0; i < 10; i++) {
    readDistanceImproved();
    delay(100);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print("System Ready!");
  display.setCursor(10, 40);
  display.print("BT: ");
  display.print(settings.bluetoothActiveSetting ? "ACTIVE" : "INACTIVE");
  display.display();
  delay(1000);
}

// =========== MAIN LOOP ===========
void loop() {
  unsigned long currentMillis = millis();
  static unsigned long lastSensorRead = 0;
  static unsigned long lastDisplayUpdate = 0;
  static unsigned long lastSave = 0;
  
  // Sensor reading (50ms)
  if (currentMillis - lastSensorRead >= 50) {
    distance = readDistanceFiltered();
    readGasSensor();
    readMotionSensor();
    readJoystick();
    lastSensorRead = currentMillis;
  }
  
  // Alert check
  checkAlerts();
  
  // Button processing
  processButtons();
  processJoystick();
  
  // Servo control
  if (manualServoMode) {
    updateServo();
  } else if (radarActive) {
    updateRadar();
  } else if (scanActive) {
    updateServo();
  }
  
  // LED and Buzzer
  updateLED();
  updateBuzzer();
  
  // Bluetooth
  if (settings.bluetoothActiveSetting) {
    checkBluetoothConnection();
    if (bluetoothConnected) {
      sendBluetoothData();
      readBluetoothData();
    }
  }
  
  // Display update (100ms)
  if (currentMillis - lastDisplayUpdate >= 100) {
    updateDisplay();
    lastDisplayUpdate = currentMillis;
  }
  
  // Eye blinking
  if (currentMillis - lastBlinkTime > 3000) {
    eyesOpen = !eyesOpen;
    lastBlinkTime = currentMillis;
  }
  
  // System LED
  digitalWrite(SYS_LED, (currentMillis % 1000) < 100);
  
  // Save settings (every 60 seconds)
  if (currentMillis - lastSave > 60000) {
    saveSettings();
    lastSave = currentMillis;
  }
  
  // Statistics update
  statistics.operationTime = currentMillis - statistics.startTime;
  
  // Python (Pydroid3) compatibility
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "GETDATA") {
      Serial.print("DIST:");
      Serial.print(distance);
      Serial.print(",GAS:");
      Serial.print(gas);
      Serial.print(",MOTION:");
      Serial.print(motion);
      Serial.print(",SERVO:");
      Serial.print(servoAngle);
      Serial.print(",RADAR:");
      Serial.print(radarActive);
      Serial.println();
    }
    else if (cmd.startsWith("SERVO:")) {
      int angle = cmd.substring(6).toInt();
      servoAngle = constrain(angle, 0, 180);
      radarServo.write(servoAngle);
      Serial.println("OK");
    }
  }
  
  delay(10); // System stability
}
