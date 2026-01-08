/****************************************************
 * TOPRAK - Akıllı Araba Asistanı PRO MAX V6.5
 * TAM PROFESYONEL KOD - 2500+ SATIR
 * 180° RADAR TARAMA + OLED KONTROL + GELİŞMİŞ MENÜ
 ****************************************************/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

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

// =========== SERVO SINIRLARI ===========
const int SERVO_MIN_LIMIT = 30;     // Sağ sınır (default)
const int SERVO_MAX_LIMIT = 150;    // Sol sınır (default)
const int SERVO_HOME_POS = 90;      // Orta nokta
const int SERVO_RANGE = 120;        // 120° tarama alanı

// =========== WEB SERVER ===========
WebServer server(80);
const char* ssid = "TOPRAK_RADAR";
const char* password = "12345678";

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
#define MENU_WEB_KONTROL 14

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
const int TOPLAM_MENU_OGE = 15;

// Uyarılar
bool uyariVar = false;
int uyariSeviye = 0;
unsigned long sonUyariZamani = 0;
bool beepCalisiyor = false;

// Robot Karakter
bool gozAcik = true;
unsigned long sonGozKirpma = 0;
int robotIfade = 0;

// Servo
int servoAci = SERVO_HOME_POS;
int hedefAci = SERVO_HOME_POS;
bool taramaYonu = false; // false = sağa, true = sola
bool taramaAktif = true;
int taramaHizi = 15;
float taramaHaritasi[37] = {0};
unsigned long sonServoHareket = 0;
bool alarmModu = false;

// LED
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

// Buzzer
unsigned long sonBip = 0;
int bipInterval = 1500;
bool bipDurum = false;

// Güvenlik
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

// Web için
unsigned long sonWebGuncelleme = 0;
bool webBagli = false;
int bagliCihazSayisi = 0;

// =========== FONKSİYON PROTOTİPLERİ ===========
void ekranCiz();
void baslikCiz();
void icerikCiz();
void anaMenuCiz();
void mesafeEkranCiz();
void gazEkranCiz();
void hareketEkranCiz();
void taramaEkranCiz();
void servoManuelCiz();
void ayarMesafeCiz();
void ayarGazCiz();
void ayarHareketCiz();
void ayarSesCiz();
void ayarServoCiz();
void ayarLedCiz();
void sistemEkranCiz();
void guvenlikEkranCiz();
void webKontrolCiz();
void altBilgiCiz();
void robotCiz(int x, int y);
void bipSesi(int tip);
void ayarDegistir(int yon);
void joyKontrol();
void joyServoKontrol();
void butonOku();
void butonIsle();
void sensorOku();
void uyariKontrol();
void servoKontrol();
void ledKontrol(unsigned long simdi);
void buzzerKontrol(unsigned long simdi);
float hcSr04Oku();
float hcSr04OkuFiltreli();
void pirOku();
void updateBlinkSpeed();
void baslangic();

// =========== WEB SAYFASI HTML ===========
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>TOPRAK PRO MAX V6.5 - Radar Kontrol</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0f2027, #203a43, #2c5364);
            color: #fff;
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        header {
            text-align: center;
            padding: 20px;
            background: rgba(0, 0, 0, 0.3);
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
        }
        
        header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            color: #00d4ff;
            text-shadow: 0 0 10px rgba(0, 212, 255, 0.5);
        }
        
        header p {
            font-size: 1.1em;
            color: #aaa;
        }
        
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-left: 10px;
            background: #00ff00;
            animation: pulse 2s infinite;
        }
        
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
            border: 1px solid rgba(255, 255, 255, 0.1);
            transition: transform 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
        }
        
        .card-header {
            font-size: 1.4em;
            font-weight: bold;
            margin-bottom: 20px;
            color: #00d4ff;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .card-header i {
            font-size: 1.2em;
        }
        
        .data-value {
            font-size: 2.8em;
            font-weight: bold;
            margin: 15px 0;
            text-shadow: 0 0 20px rgba(0, 212, 255, 0.5);
        }
        
        .data-label {
            font-size: 1.1em;
            color: #aaa;
            margin-top: 5px;
        }
        
        .unit {
            font-size: 0.6em;
            color: #00d4ff;
        }
        
        .warning {
            color: #ff9900;
        }
        
        .danger {
            color: #ff3300;
            animation: blink 1s infinite;
        }
        
        @keyframes blink {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        .safe {
            color: #00ff00;
        }
        
        .gauge-container {
            width: 100%;
            height: 150px;
            position: relative;
            margin: 20px 0;
        }
        
        #radarCanvas {
            background: rgba(0, 0, 0, 0.3);
            border-radius: 10px;
            width: 100%;
            height: 100%;
        }
        
        .controls {
            display: flex;
            gap: 15px;
            margin-top: 20px;
            flex-wrap: wrap;
        }
        
        .control-group {
            flex: 1;
            min-width: 200px;
        }
        
        .control-label {
            display: block;
            margin-bottom: 8px;
            color: #aaa;
        }
        
        input[type="range"] {
            width: 100%;
            height: 8px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 4px;
            outline: none;
            -webkit-appearance: none;
        }
        
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 20px;
            height: 20px;
            background: #00d4ff;
            border-radius: 50%;
            cursor: pointer;
            box-shadow: 0 0 10px rgba(0, 212, 255, 0.5);
        }
        
        button {
            background: linear-gradient(45deg, #0066cc, #0099ff);
            color: white;
            border: none;
            padding: 12px 25px;
            border-radius: 8px;
            cursor: pointer;
            font-size: 1em;
            font-weight: bold;
            transition: all 0.3s ease;
            flex: 1;
        }
        
        button:hover {
            background: linear-gradient(45deg, #0055aa, #0088ee);
            transform: scale(1.05);
            box-shadow: 0 0 15px rgba(0, 212, 255, 0.3);
        }
        
        button:active {
            transform: scale(0.95);
        }
        
        button.danger {
            background: linear-gradient(45deg, #cc0000, #ff3300);
        }
        
        button.success {
            background: linear-gradient(45deg, #00cc66, #00ff88);
        }
        
        button.warning {
            background: linear-gradient(45deg, #cc9900, #ffbb33);
        }
        
        .alarm-panel {
            background: rgba(255, 0, 0, 0.1);
            border: 2px solid rgba(255, 0, 0, 0.3);
            padding: 15px;
            border-radius: 10px;
            margin-top: 15px;
        }
        
        .system-info {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }
        
        .info-item {
            background: rgba(255, 255, 255, 0.05);
            padding: 10px;
            border-radius: 8px;
            text-align: center;
        }
        
        .info-value {
            font-size: 1.8em;
            font-weight: bold;
            color: #00d4ff;
        }
        
        .info-label {
            font-size: 0.9em;
            color: #aaa;
            margin-top: 5px;
        }
        
        .log-container {
            max-height: 200px;
            overflow-y: auto;
            background: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
            padding: 10px;
            margin-top: 15px;
        }
        
        .log-entry {
            padding: 5px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
            font-family: monospace;
            font-size: 0.9em;
        }
        
        .log-time {
            color: #00d4ff;
            margin-right: 10px;
        }
        
        .log-message {
            color: #fff;
        }
        
        .servo-indicator {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin: 15px 0;
            padding: 10px;
            background: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
        }
        
        .servo-angle {
            font-size: 2em;
            font-weight: bold;
            color: #00d4ff;
        }
        
        .connection-status {
            text-align: center;
            padding: 10px;
            margin-top: 20px;
            border-radius: 8px;
            background: rgba(0, 0, 0, 0.3);
        }
        
        @media (max-width: 768px) {
            .grid {
                grid-template-columns: 1fr;
            }
            
            .data-value {
                font-size: 2.2em;
            }
            
            button {
                padding: 10px 15px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>📡 TOPRAK PRO MAX V6.5</h1>
            <p>180° Radar Tarama Sistemi | Gerçek Zamanlı İzleme</p>
            <div class="connection-status">
                <span>Bağlantı Durumu: <span id="connStatus">BAĞLI</span></span>
                <span class="status-indicator"></span>
            </div>
        </header>
        
        <div class="grid">
            <div class="card">
                <div class="card-header">🎯 Radar Kontrol</div>
                <div class="gauge-container">
                    <canvas id="radarCanvas" width="300" height="150"></canvas>
                </div>
                <div class="servo-indicator">
                    <span>Servo Açısı:</span>
                    <span class="servo-angle" id="servoAngle">90°</span>
                </div>
                <div class="controls">
                    <div class="control-group">
                        <label class="control-label">Servo Açısı:</label>
                        <input type="range" id="servoSlider" min="30" max="150" value="90" step="1">
                    </div>
                    <button id="centerBtn">Orta Nokta</button>
                    <button id="scanBtn" class="success">Tarama Başlat</button>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">📊 Sensör Verileri</div>
                <div>
                    <div class="data-value" id="distanceValue">0</div>
                    <div class="data-label">Mesafe <span class="unit">(cm)</span></div>
                    
                    <div class="data-value" id="gasValue">0</div>
                    <div class="data-label">Gaz Seviyesi <span class="unit">(ppm)</span></div>
                    
                    <div class="data-value" id="motionValue">HAYIR</div>
                    <div class="data-label">Hareket Algılandı</div>
                </div>
                <div class="alarm-panel" id="alarmPanel" style="display: none;">
                    <div class="data-value danger">⚠️ UYARI!</div>
                    <div class="data-label" id="alarmMessage"></div>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">⚙️ Sistem Kontrol</div>
                <div class="controls">
                    <button id="buzzerBtn" class="warning">Buzzer Test</button>
                    <button id="ledBtn" class="success">LED Aç/Kapa</button>
                    <button id="securityBtn" class="danger">Güvenlik Sistemi</button>
                </div>
                
                <div class="system-info">
                    <div class="info-item">
                        <div class="info-value" id="heapValue">0</div>
                        <div class="info-label">Hafıza (KB)</div>
                    </div>
                    <div class="info-item">
                        <div class="info-value" id="uptimeValue">0</div>
                        <div class="info-label">Çalışma Süresi</div>
                    </div>
                    <div class="info-item">
                        <div class="info-value" id="wifiValue">0</div>
                        <div class="info-label">WiFi Gücü</div>
                    </div>
                </div>
                
                <div class="log-container" id="logContainer">
                    <div class="log-entry"><span class="log-time">00:00</span><span class="log-message">Sistem başlatıldı</span></div>
                </div>
            </div>
        </div>
    </div>

    <script>
        let servoAngle = 90;
        let distanceValue = 0;
        let gasValue = 0;
        let motionValue = false;
        let securityActive = false;
        let ledState = true;
        let lastUpdate = 0;
        let logEntries = [];
        
        // DOM Elements
        const servoSlider = document.getElementById('servoSlider');
        const servoAngleDisplay = document.getElementById('servoAngle');
        const distanceDisplay = document.getElementById('distanceValue');
        const gasDisplay = document.getElementById('gasValue');
        const motionDisplay = document.getElementById('motionValue');
        const alarmPanel = document.getElementById('alarmPanel');
        const alarmMessage = document.getElementById('alarmMessage');
        const connStatus = document.getElementById('connStatus');
        const logContainer = document.getElementById('logContainer');
        const radarCanvas = document.getElementById('radarCanvas');
        const ctx = radarCanvas.getContext('2d');
        
        // System displays
        const heapDisplay = document.getElementById('heapValue');
        const uptimeDisplay = document.getElementById('uptimeValue');
        const wifiDisplay = document.getElementById('wifiValue');
        
        // Buttons
        const centerBtn = document.getElementById('centerBtn');
        const scanBtn = document.getElementById('scanBtn');
        const buzzerBtn = document.getElementById('buzzerBtn');
        const ledBtn = document.getElementById('ledBtn');
        const securityBtn = document.getElementById('securityBtn');
        
        // Radar data
        let radarPoints = [];
        let currentAngle = 90;
        
        // Initialize
        function init() {
            addLog("Web arayüzü başlatıldı");
            updateData();
            drawRadar();
            
            // Event listeners
            servoSlider.addEventListener('input', function() {
                servoAngle = parseInt(this.value);
                servoAngleDisplay.textContent = servoAngle + '°';
                setServoAngle(servoAngle);
            });
            
            centerBtn.addEventListener('click', function() {
                servoSlider.value = 90;
                servoAngle = 90;
                servoAngleDisplay.textContent = '90°';
                setServoAngle(90);
                addLog("Servo orta noktaya alındı");
            });
            
            scanBtn.addEventListener('click', function() {
                toggleScan();
            });
            
            buzzerBtn.addEventListener('click', function() {
                testBuzzer();
            });
            
            ledBtn.addEventListener('click', function() {
                toggleLED();
            });
            
            securityBtn.addEventListener('click', function() {
                toggleSecurity();
            });
            
            // Auto-update
            setInterval(updateData, 1000);
            setInterval(drawRadar, 100);
        }
        
        // Update sensor data
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    lastUpdate = Date.now();
                    
                    // Update displays
                    distanceValue = data.distance;
                    distanceDisplay.textContent = distanceValue.toFixed(1);
                    distanceDisplay.className = 'data-value ' + getDistanceClass(distanceValue);
                    
                    gasValue = data.gas;
                    gasDisplay.textContent = gasValue.toFixed(0);
                    gasDisplay.className = 'data-value ' + getGasClass(gasValue);
                    
                    motionValue = data.motion;
                    motionDisplay.textContent = motionValue ? 'EVET' : 'HAYIR';
                    motionDisplay.className = 'data-value ' + (motionValue ? 'warning' : 'safe');
                    
                    // Update servo
                    servoAngle = data.servoAngle;
                    servoSlider.value = servoAngle;
                    servoAngleDisplay.textContent = servoAngle + '°';
                    currentAngle = servoAngle;
                    
                    // Update alarm
                    if (data.alarm) {
                        alarmPanel.style.display = 'block';
                        alarmMessage.textContent = data.alarmMessage;
                    } else {
                        alarmPanel.style.display = 'none';
                    }
                    
                    // Update system info
                    heapDisplay.textContent = Math.round(data.heap / 1024);
                    uptimeDisplay.textContent = formatUptime(data.uptime);
                    wifiDisplay.textContent = data.wifi + ' dBm';
                    
                    // Add radar point
                    if (distanceValue > 2 && distanceValue < 400) {
                        radarPoints.push({
                            angle: currentAngle,
                            distance: distanceValue,
                            timestamp: Date.now()
                        });
                        
                        // Keep only recent points
                        if (radarPoints.length > 100) {
                            radarPoints.shift();
                        }
                    }
                    
                    // Update connection status
                    if (Date.now() - lastUpdate < 5000) {
                        connStatus.textContent = 'BAĞLI';
                        connStatus.style.color = '#00ff00';
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    connStatus.textContent = 'KOPUK';
                    connStatus.style.color = '#ff0000';
                });
        }
        
        // Draw radar
        function drawRadar() {
            const width = radarCanvas.width;
            const height = radarCanvas.height;
            const centerX = width / 2;
            const centerY = height - 20;
            const maxRadius = Math.min(centerX, centerY) - 10;
            
            // Clear canvas
            ctx.clearRect(0, 0, width, height);
            
            // Draw background circles
            ctx.strokeStyle = 'rgba(0, 212, 255, 0.3)';
            ctx.lineWidth = 1;
            
            for (let i = 1; i <= 4; i++) {
                const radius = (maxRadius / 4) * i;
                ctx.beginPath();
                ctx.arc(centerX, centerY, radius, 0, Math.PI, false);
                ctx.stroke();
            }
            
            // Draw angle lines
            ctx.strokeStyle = 'rgba(255, 255, 255, 0.2)';
            for (let angle = 30; angle <= 150; angle += 30) {
                const rad = (angle - 90) * Math.PI / 180;
                const x = centerX + Math.sin(rad) * maxRadius;
                const y = centerY - Math.cos(rad) * maxRadius;
                
                ctx.beginPath();
                ctx.moveTo(centerX, centerY);
                ctx.lineTo(x, y);
                ctx.stroke();
            }
            
            // Draw radar line
            const currentRad = (currentAngle - 90) * Math.PI / 180;
            ctx.strokeStyle = '#00ff00';
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(centerX, centerY);
            ctx.lineTo(
                centerX + Math.sin(currentRad) * maxRadius,
                centerY - Math.cos(currentRad) * maxRadius
            );
            ctx.stroke();
            
            // Draw points
            for (const point of radarPoints) {
                const age = Date.now() - point.timestamp;
                if (age > 10000) continue; // Remove old points
                
                const alpha = 1 - (age / 10000);
                const pointRad = (point.angle - 90) * Math.PI / 180;
                const distance = Math.min(point.distance, 400);
                const radius = (distance / 400) * maxRadius;
                
                const x = centerX + Math.sin(pointRad) * radius;
                const y = centerY - Math.cos(pointRad) * radius;
                
                // Color based on distance
                let color;
                if (distance < 50) color = '#ff0000';
                else if (distance < 100) color = '#ff9900';
                else color = '#00ff00';
                
                ctx.fillStyle = color.replace(')', `, ${alpha})`).replace('rgb', 'rgba');
                ctx.beginPath();
                ctx.arc(x, y, 3, 0, Math.PI * 2);
                ctx.fill();
            }
            
            // Draw labels
            ctx.fillStyle = '#ffffff';
            ctx.font = '12px Arial';
            ctx.textAlign = 'center';
            
            // Distance labels
            for (let i = 1; i <= 4; i++) {
                const radius = (maxRadius / 4) * i;
                const label = (i * 100) + 'cm';
                ctx.fillText(label, centerX, centerY - radius - 5);
            }
            
            // Angle labels
            ctx.fillStyle = 'rgba(255, 255, 255, 0.7)';
            for (let angle = 30; angle <= 150; angle += 30) {
                const rad = (angle - 90) * Math.PI / 180;
                const x = centerX + Math.sin(rad) * (maxRadius + 15);
                const y = centerY - Math.cos(rad) * (maxRadius + 15);
                ctx.fillText(angle + '°', x, y + 4);
            }
        }
        
        // Control functions
        function setServoAngle(angle) {
            fetch('/servo?angle=' + angle)
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        addLog("Servo " + angle + "° açısına ayarlandı");
                    }
                });
        }
        
        function toggleScan() {
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    const button = scanBtn;
                    if (data.scanning) {
                        button.textContent = 'Tarama Durdur';
                        button.classList.remove('success');
                        button.classList.add('danger');
                        addLog("Tarama başlatıldı");
                    } else {
                        button.textContent = 'Tarama Başlat';
                        button.classList.remove('danger');
                        button.classList.add('success');
                        addLog("Tarama durduruldu");
                    }
                });
        }
        
        function testBuzzer() {
            fetch('/buzzer')
                .then(response => {
                    addLog("Buzzer test edildi");
                });
        }
        
        function toggleLED() {
            fetch('/led')
                .then(response => response.json())
                .then(data => {
                    ledState = data.ledOn;
                    const button = ledBtn;
                    if (ledState) {
                        button.textContent = 'LED Kapat';
                        button.classList.remove('success');
                        button.classList.add('warning');
                        addLog("LED açıldı");
                    } else {
                        button.textContent = 'LED Aç';
                        button.classList.remove('warning');
                        button.classList.add('success');
                        addLog("LED kapatıldı");
                    }
                });
        }
        
        function toggleSecurity() {
            fetch('/security')
                .then(response => response.json())
                .then(data => {
                    securityActive = data.securityActive;
                    const button = securityBtn;
                    if (securityActive) {
                        button.textContent = 'Güvenlik Kapat';
                        button.classList.remove('danger');
                        button.classList.add('success');
                        addLog("Güvenlik sistemi aktif");
                    } else {
                        button.textContent = 'Güvenlik Aç';
                        button.classList.remove('success');
                        button.classList.add('danger');
                        addLog("Güvenlik sistemi pasif");
                    }
                });
        }
        
        // Helper functions
        function getDistanceClass(distance) {
            if (distance < 20) return 'danger';
            if (distance < 50) return 'warning';
            return 'safe';
        }
        
        function getGasClass(gas) {
            if (gas > 1000) return 'danger';
            if (gas > 500) return 'warning';
            return 'safe';
        }
        
        function formatUptime(seconds) {
            const hours = Math.floor(seconds / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);
            return hours + 'h ' + minutes + 'm';
        }
        
        function addLog(message) {
            const now = new Date();
            const time = ('0' + now.getHours()).slice(-2) + ':' + ('0' + now.getMinutes()).slice(-2);
            
            const logEntry = document.createElement('div');
            logEntry.className = 'log-entry';
            logEntry.innerHTML = `<span class="log-time">${time}</span><span class="log-message">${message}</span>`;
            
            logContainer.insertBefore(logEntry, logContainer.firstChild);
            
            // Keep only last 20 entries
            while (logContainer.children.length > 20) {
                logContainer.removeChild(logContainer.lastChild);
            }
        }
        
        // Start
        init();
    </script>
</body>
</html>
)rawliteral";

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
  
  for (int i = 0; i < 5; i++) {
    float okuma = hcSr04Oku();
    
    if (okuma >= 2.0 && okuma <= 400.0) {
      if (i > 0 && abs(okuma - toplam/gecerliOkuma) > 100.0) {
        // 100cm'den fazla ani değişim, geçersiz say
        continue;
      }
      
      toplam += okuma;
      gecerliOkuma++;
    }
    delay(10);
  }
  
  if (gecerliOkuma > 0) {
    float ortalama = toplam / gecerliOkuma;
    
    mesafeBuffer[bufferIndex] = ortalama;
    bufferIndex = (bufferIndex + 1) % MESAFE_BUFFER_SIZE;
    
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
  
  if (servoAci < ayar.servoMinAci) {
    servoAci = ayar.servoMinAci;
    Serial.println("SERVO SAG SINIRA ULASILDI");
  } else if (servoAci > ayar.servoMaxAci) {
    servoAci = ayar.servoMaxAci;
    Serial.println("SERVO SOL SINIRA ULASILDI");
  }
  
  if (servoAci != hedefAci) {
    if (servoAci < hedefAci) {
      servoAci++;
    } else {
      servoAci--;
    }
    
    taramaServo.write(servoAci);
  } else {
    if (!taramaYonu) {
      hedefAci += 10;
      if (hedefAci > ayar.servoMaxAci) {
        hedefAci = ayar.servoMaxAci;
        taramaYonu = true;
        
        if (ayar.ses) {
          tone(BUZZER_PIN, 800, 50);
        }
        Serial.println("TARAMA YONU DEGISTI: SOLA");
      }
    } else {
      hedefAci -= 10;
      if (hedefAci < ayar.servoMinAci) {
        hedefAci = ayar.servoMinAci;
        taramaYonu = false;
        
        if (ayar.ses) {
          tone(BUZZER_PIN, 1200, 50);
        }
        Serial.println("TARAMA YONU DEGISTI: SAGA");
      }
    }
    
    hedefAci = constrain(hedefAci, ayar.servoMinAci, ayar.servoMaxAci);
  }
  
  int taramaIndex = map(servoAci, ayar.servoMinAci, ayar.servoMaxAci, 0, 36);
  taramaIndex = constrain(taramaIndex, 0, 36);
  
  if (mesafe >= 2.0 && mesafe <= 400.0) {
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
        tone(BUZZER_PIN, 800, 50);
      }
      Serial.println("SERVO SAG SINIRA ULASILDI (JOY)");
    }
    taramaServo.write(servoAci);
    sonJoyServoHareket = simdi;
  } else if (joyX > 3000) {
    servoAci += 5;
    if (servoAci > ayar.servoMaxAci) {
      servoAci = ayar.servoMaxAci;
      if (ayar.ses) {
        tone(BUZZER_PIN, 1200, 50);
      }
      Serial.println("SERVO SOL SINIRA ULASILDI (JOY)");
    }
    taramaServo.write(servoAci);
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

// =========== EKRAN FONKSİYONLARI - TAMAMI EKLENDİ ===========
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
    case MENU_WEB_KONTROL: display.print("WEB KONTROL"); break;
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
    case MENU_WEB_KONTROL: webKontrolCiz(); break;
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
    "Web Kontrol"
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
  display.print("TOPRAK PRO MAX V6.5");
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

void webKontrolCiz() {
  display.setTextSize(1);
  display.setCursor(5, 15);
  display.print("Web Kontrol Aktif");
  
  display.setCursor(5, 25);
  display.print("IP: ");
  display.print(WiFi.localIP());
  
  display.setCursor(5, 35);
  display.print("Bagli Cihaz:");
  display.print(bagliCihazSayisi);
  
  display.setCursor(5, 45);
  display.print("Mesafe: ");
  display.print(mesafe, 1);
  display.print("cm");
  
  display.setCursor(5, 55);
  display.print("Tarama: ");
  display.print(taramaAktif ? "AKTIF" : "PASIF");
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
    display.print("UYARI! M:");
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
          case 13: aktifMenu = MENU_WEB_KONTROL; break;
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

// =========== WEB SERVER HANDLERS ===========
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleData() {
  StaticJsonDocument<512> doc;
  
  doc["distance"] = mesafe;
  doc["gas"] = gaz;
  doc["motion"] = hareket;
  doc["servoAngle"] = servoAci;
  doc["scanning"] = taramaAktif;
  doc["ledOn"] = (ledMod != 0 || ledDurum);
  doc["securityActive"] = guvenlikAktif;
  doc["heap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  doc["wifi"] = WiFi.RSSI();
  doc["servoMin"] = ayar.servoMinAci;
  doc["servoMax"] = ayar.servoMaxAci;
  
  if (uyariVar) {
    doc["alarm"] = true;
    if (mesafe < ayar.mesafeTehlike) {
      doc["alarmMessage"] = "Tehlikeli mesafe: " + String(mesafe, 1) + "cm";
    } else if (mesafe < ayar.mesafeDikkat) {
      doc["alarmMessage"] = "Dikkat! Mesafe: " + String(mesafe, 1) + "cm";
    } else if (gaz > ayar.gazEsik) {
      doc["alarmMessage"] = "Yüksek gaz seviyesi: " + String(gaz, 0) + "ppm";
    }
  } else {
    doc["alarm"] = false;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleServo() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    angle = constrain(angle, ayar.servoMinAci, ayar.servoMaxAci);
    
    taramaAktif = false;
    servoAci = angle;
    taramaServo.write(servoAci);
    
    StaticJsonDocument<200> doc;
    doc["success"] = true;
    doc["angle"] = servoAci;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  } else {
    server.send(400, "text/plain", "Missing angle parameter");
  }
}

void handleScan() {
  taramaAktif = !taramaAktif;
  
  StaticJsonDocument<200> doc;
  doc["success"] = true;
  doc["scanning"] = taramaAktif;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleBuzzer() {
  tone(BUZZER_PIN, 1500, 200);
  server.send(200, "text/plain", "OK");
}

void handleLED() {
  ayar.ledAktif = !ayar.ledAktif;
  
  StaticJsonDocument<200> doc;
  doc["success"] = true;
  doc["ledOn"] = ayar.ledAktif;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSecurity() {
  guvenlikAktif = !guvenlikAktif;
  
  StaticJsonDocument<200> doc;
  doc["success"] = true;
  doc["securityActive"] = guvenlikAktif;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// =========== KURULUM ===========
void setup() {
  Serial.begin(115200);
  Serial.println("TOPRAK PRO MAX V6.5 baslatiliyor...");
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
  
  // WiFi başlatma
  Serial.println("WiFi baslatiliyor...");
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  
  // Web server route'ları
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/servo", handleServo);
  server.on("/scan", handleScan);
  server.on("/buzzer", handleBuzzer);
  server.on("/led", handleLED);
  server.on("/security", handleSecurity);
  
  server.begin();
  Serial.println("HTTP server baslatildi");
  
  Serial.println("TOPRAK PRO MAX V6.5 - Sistem hazir!");
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
  display.print("PRO MAX V6.5");
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
  
  // Web server istemci sayısını güncelle
  bagliCihazSayisi = WiFi.softAPgetStationNum();
  
  // Web server'ı işle
  server.handleClient();
  
  delay(10);
}
