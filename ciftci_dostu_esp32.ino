#include <WiFi.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// MPU6050 Kütüphaneleri
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h> // I2C iletişimi için gerekli

// --- Pin Tanımlamaları ---
// Sensörler
#define TRIG_PIN 12          // HC-SR04 Trig Pini
#define ECHO_PIN 13          // HC-SR04 Echo Pini
#define DHTPIN 15            // DHT22 Data Pini
#define SOIL_MOISTURE_PIN 32 // Toprak Nem Sensörü Analog Pini (ADC1_CH4)
#define BATTERY_PIN 34       // Batarya Voltaj Okuma Pini (ADC1_CH6)

// MPU6050 I2C pinleri ESP32'de varsayılan olarak: SDA = 21, SCL = 22

// Motor Kontrol Pinleri (L298N veya benzeri bir sürücü için)
// Sol Motor
#define MOTOR_L_EN_PIN 25  // Sol Motor Enable (PWM Hız Kontrolü)
#define MOTOR_L_IN1_PIN 26 // Sol Motor Giriş 1
#define MOTOR_L_IN2_PIN 27 // Sol Motor Giriş 2
// Sağ Motor
#define MOTOR_R_EN_PIN 33  // Sağ Motor Enable (PWM Hız Kontrolü)
#define MOTOR_R_IN3_PIN 18 // Sağ Motor Giriş 3
#define MOTOR_R_IN4_PIN 19 // Sağ Motor Giriş 4

// LED Pinleri
#define LED_WIFI_STATUS_PIN 2 // WiFi bağlantı durumunu gösteren LED
#define LED_MOVEMENT_PIN 4    // Araç hareket halindeyken yanan LED
#define LED_LOW_BATTERY_PIN 5 // Düşük batarya uyarısı için LED

// Buzzer Pini
#define BUZZER_PIN 14

// --- WiFi ve Web Sunucusu Ayarları ---
const char *ssid = "YOUR_WIFI_NAME";    // KENDİ WIFI ADINIZI GİRİN
const char *password = "YOUR_PASSWORD"; // KENDİ WIFI ŞİFRENİZİ GİRİN
WiFiServer server(80);

// --- Sensör Nesneleri ---
DHT dht(DHTPIN, DHT22);
TinyGPSPlus gps;
HardwareSerial SerialGPS(2); // UART2 (GPS için: RX2=GPIO16, TX2=GPIO17 varsayılan)
Adafruit_MPU6050 mpu;

// --- Batarya Ayarları ---
float batteryVoltage = 0.0;
const float LOW_BATTERY_THRESHOLD = 11.0; // Düşük batarya eşiği (V) - Örnek değer, kendi bataryanıza göre ayarlayın
bool lowBatteryAlarmActive = false;

// --- Motor PWM Ayarları ---
const int PWM_FREQ = 5000;    // PWM Frekansı (Hz)
const int PWM_RESOLUTION = 8; // PWM Çözünürlüğü (bit) - 0-255 arası değerler
// PWM Kanalları (ESP32'de 16 kanal var, 0-15)
const int PWM_CHANNEL_L = 0; // Sol motor için PWM kanalı
const int PWM_CHANNEL_R = 1; // Sağ motor için PWM kanalı

// Varsayılan motor hızı (0-255)
int motorSpeed = 200;

// --- Global Değişkenler ---
float mpu_accX = 0, mpu_accY = 0, mpu_accZ = 0;
float mpu_gyroX = 0, mpu_gyroY = 0, mpu_gyroZ = 0;
float mpu_temp = 0;
int soilMoistureValue = 0;

void setup()
{
  Serial.begin(9600); // Raspberry Pi ile iletişim ve hata ayıklama için
  Serial.println("ESP32 Cihazi Baslatiliyor (v2)...");

  // Pin Modları
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT); // Analog giriş
  pinMode(BATTERY_PIN, INPUT);

  pinMode(LED_WIFI_STATUS_PIN, OUTPUT);
  pinMode(LED_MOVEMENT_PIN, OUTPUT);
  pinMode(LED_LOW_BATTERY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_WIFI_STATUS_PIN, LOW); // Başlangıçta LED'ler sönük
  digitalWrite(LED_MOVEMENT_PIN, LOW);
  digitalWrite(LED_LOW_BATTERY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Motor Pinlerini Ayarla
  pinMode(MOTOR_L_IN1_PIN, OUTPUT);
  pinMode(MOTOR_L_IN2_PIN, OUTPUT);
  pinMode(MOTOR_R_IN3_PIN, OUTPUT);
  pinMode(MOTOR_R_IN4_PIN, OUTPUT);

  // Motor PWM Kanallarını Ayarla
  ledcSetup(PWM_CHANNEL_L, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_L_EN_PIN, PWM_CHANNEL_L);
  ledcSetup(PWM_CHANNEL_R, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_R_EN_PIN, PWM_CHANNEL_R);

  stopMotors(); // Başlangıçta motorlar dursun

  // DHT Sensörünü Başlatma
  dht.begin();
  Serial.println("DHT22 Sensoru Baslatildi.");

  // MPU6050 Başlatma
  Wire.begin(); // I2C başlat (SDA, SCL pinleri belirtilmezse varsayılan kullanılır)
  if (!mpu.begin())
  {
    Serial.println("MPU6050 algilanamadi! Baglantilari kontrol edin.");
    // while (1) delay(10); // MPU6050 bulunamazsa burada kal (isteğe bağlı)
  }
  else
  {
    Serial.println("MPU6050 Baslatildi.");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DPS);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  // GPS Modülü için Seri Port Başlatma
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17); // GPS baud, RX2, TX2
  Serial.println("GPS Seri Portu (Serial2) Baslatildi.");

  // WiFi Bağlantısı
  digitalWrite(LED_WIFI_STATUS_PIN, HIGH); // Bağlanmaya çalışırken LED'i yak
  Serial.print("WiFi Agina Baglaniliyor: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int wifi_retries = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_retries < 20)
  {                                                                       // 20 deneme (10 saniye)
    digitalWrite(LED_WIFI_STATUS_PIN, !digitalRead(LED_WIFI_STATUS_PIN)); // LED'i yanıp söndür
    delay(500);
    Serial.print(".");
    wifi_retries++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LED_WIFI_STATUS_PIN, HIGH); // Bağlantı başarılıysa LED sabit yansın
    Serial.println("\nWiFi Baglantisi Basarili!");
    Serial.print("IP Adresi: ");
    Serial.println(WiFi.localIP());
    server.begin(); // Web Sunucusunu Başlatma
    Serial.println("Web Sunucusu Baslatildi.");
  }
  else
  {
    digitalWrite(LED_WIFI_STATUS_PIN, LOW); // Bağlantı başarısızsa LED'i söndür
    Serial.println("\nWiFi Baglantisi Basarisiz!");
  }

  Serial.println("Kurulum Tamamlandi. Cihaz Hazir.");
}

// --- Sensör Okuma Fonksiyonları ---
float readDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms timeout
  return duration * 0.0343 / 2.0;
}

float readBatteryVoltage()
{
  int adcValue = analogRead(BATTERY_PIN);
  float voltageDividerFactor = 2.0; // Kendi voltaj bölücünüze göre ayarlayın
  return adcValue * (3.3 / 4095.0) * voltageDividerFactor;
}

void readMPU6050Data()
{
  if (mpu.getMotionInterruptStatus())
  { // Veri hazır mı kontrol et (isteğe bağlı)
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    mpu_accX = a.acceleration.x;
    mpu_accY = a.acceleration.y;
    mpu_accZ = a.acceleration.z;
    mpu_gyroX = g.gyro.x;
    mpu_gyroY = g.gyro.y;
    mpu_gyroZ = g.gyro.z;
    mpu_temp = temp.temperature;
  }
  else
  {
    // Eğer getMotionInterruptStatus her zaman 0 dönüyorsa veya kullanmıyorsanız direkt okuyun:
    sensors_event_t a, g, temp_event; // temp_event olarak değiştirdim, global mpu_temp ile karışmasın diye
    mpu.getEvent(&a, &g, &temp_event);
    mpu_accX = a.acceleration.x;
    mpu_accY = a.acceleration.y;
    mpu_accZ = a.acceleration.z;
    mpu_gyroX = g.gyro.x;
    mpu_gyroY = g.gyro.y;
    mpu_gyroZ = g.gyro.z;
    mpu_temp = temp_event.temperature;
  }
}

void readSoilMoisture()
{
  soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  // Okunan değeri yüzdeye veya başka bir birime çevirebilirsiniz.
  // Örneğin: soilMoisturePercent = map(soilMoistureValue, 0, 4095, 100, 0); // Kuru=0, Islak=4095 için
}

// --- Motor Kontrol Fonksiyonları ---
void setMotor(int motorNum, bool forward, int speedVal)
{
  // motorNum: 0 = Sol, 1 = Sağ
  // forward: true = İleri, false = Geri
  // speedVal: 0-255 arası hız değeri

  if (motorNum == 0)
  { // Sol Motor
    digitalWrite(MOTOR_L_IN1_PIN, forward ? HIGH : LOW);
    digitalWrite(MOTOR_L_IN2_PIN, forward ? LOW : HIGH);
    ledcWrite(PWM_CHANNEL_L, speedVal);
  }
  else if (motorNum == 1)
  { // Sağ Motor
    digitalWrite(MOTOR_R_IN3_PIN, forward ? HIGH : LOW);
    digitalWrite(MOTOR_R_IN4_PIN, forward ? LOW : HIGH);
    ledcWrite(PWM_CHANNEL_R, speedVal);
  }
  digitalWrite(LED_MOVEMENT_PIN, (speedVal > 0)); // Hız varsa hareket LED'ini yak
}

void moveForward(int speedVal)
{
  setMotor(0, true, speedVal); // Sol ileri
  setMotor(1, true, speedVal); // Sağ ileri
  Serial.println("Motorlar: ILERI");
}

void moveBackward(int speedVal)
{
  setMotor(0, false, speedVal); // Sol geri
  setMotor(1, false, speedVal); // Sağ geri
  Serial.println("Motorlar: GERI");
}

void turnLeft(int speedVal)
{
  setMotor(0, false, speedVal); // Sol geri (veya durdur/daha yavaş ileri)
  setMotor(1, true, speedVal);  // Sağ ileri
  Serial.println("Motorlar: SOLA DONUS");
}

void turnRight(int speedVal)
{
  setMotor(0, true, speedVal);  // Sol ileri
  setMotor(1, false, speedVal); // Sağ geri (veya durdur/daha yavaş ileri)
  Serial.println("Motorlar: SAGA DONUS");
}

void stopMotors()
{
  setMotor(0, true, 0); // Sol dur
  setMotor(1, true, 0); // Sağ dur
  Serial.println("Motorlar: DUR");
}

// --- Uyarı Fonksiyonları ---
void checkBattery()
{
  batteryVoltage = readBatteryVoltage();
  if (batteryVoltage < LOW_BATTERY_THRESHOLD && batteryVoltage > 5.0)
  { // 5V altı hatalı okuma olabilir
    if (!lowBatteryAlarmActive)
    {
      Serial.println("UYARI: Batarya Seviyesi Dusuk!");
      digitalWrite(LED_LOW_BATTERY_PIN, HIGH);
      // Sesli uyarı (örneğin 3 kısa bip)
      for (int i = 0; i < 3; i++)
      {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
      }
      lowBatteryAlarmActive = true;
      // Burada SMS gönderme fonksiyonu çağrılabilir (GSM modülü varsa)
      // sendLowBatterySMS();
    }
  }
  else
  {
    digitalWrite(LED_LOW_BATTERY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW); // Sürekli çalmasını engelle
    lowBatteryAlarmActive = false;
  }
}

void loop()
{
  // GPS Verilerini İşleme
  while (SerialGPS.available() > 0)
  {
    if (gps.encode(SerialGPS.read()))
    {
      // GPS verisi başarıyla çözüldü
    }
  }

  // Sensör Verilerini Oku
  readMPU6050Data();
  readSoilMoisture();
  checkBattery(); // Batarya durumunu kontrol et ve uyar

  // Raspberry Pi'den Gelen Seri Komutları Kontrol Etme
  if (Serial.available() > 0)
  {
    String commandFromPi = Serial.readStringUntil('\n');
    commandFromPi.trim();
    Serial.print("Raspberry Pi'den Gelen Komut: ");
    Serial.println(commandFromPi);

    if (commandFromPi.equalsIgnoreCase("forward"))
    {
      moveForward(motorSpeed);
    }
    else if (commandFromPi.equalsIgnoreCase("stop"))
    {
      stopMotors();
    }
    else if (commandFromPi.equalsIgnoreCase("back"))
    { // Pi'den geri komutu gelirse
      moveBackward(motorSpeed);
    }
    else if (commandFromPi.equalsIgnoreCase("left"))
    { // Pi'den sol komutu gelirse
      turnLeft(motorSpeed);
    }
    else if (commandFromPi.equalsIgnoreCase("right"))
    { // Pi'den sağ komutu gelirse
      turnRight(motorSpeed);
    }
    // Hız ayarı için komut eklenebilir: "speed:150" gibi
  }

  // Web Sunucusu İstemcilerini Kontrol Etme (Eğer WiFi bağlıysa)
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClient client = server.available();
    if (client)
    {
      Serial.println("Yeni Web Istemcisi Baglandi.");
      String currentLine = "";
      String requestPath = ""; // İstek yolunu saklamak için
      unsigned long startTime = millis();

      while (client.connected() && (millis() - startTime < 2000))
      { // 2 saniye timeout
        if (client.available())
        {
          char c = client.read();
          // Serial.write(c); // Gelen tüm isteği seri monitöre yazdır (debug için)
          if (c == '\n')
          {
            if (currentLine.length() == 0)
            { // HTTP isteği boş bir satırla biter
              // HTTP başlıklarını gönder
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              // URL'ye göre motorları kontrol et
              if (requestPath.indexOf("/forward") >= 0)
                moveForward(motorSpeed);
              else if (requestPath.indexOf("/back") >= 0)
                moveBackward(motorSpeed);
              else if (requestPath.indexOf("/left") >= 0)
                turnLeft(motorSpeed);
              else if (requestPath.indexOf("/right") >= 0)
                turnRight(motorSpeed);
              else if (requestPath.indexOf("/stop") >= 0)
                stopMotors();
              // Hız ayarı için: /speed?val=150 gibi bir URL işlenebilir.
              // Örneğin: if (requestPath.indexOf("/speed") >=0) {
              //   int valStart = requestPath.indexOf("val=") + 4;
              //   if (valStart > 3) motorSpeed = requestPath.substring(valStart).toInt();
              // }

              // --- HTML Sayfasını Gönder ---
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
              client.println("<title>Ciftci Dostu Kontrol Paneli v2</title>");
              client.println("<style>body{font-family:Arial,sans-serif;margin:0;padding:10px;background-color:#e8f5e9;color:#1b5e20;}");
              client.println(".container{max-width:600px;margin:auto;background-color:#fff;padding:20px;border-radius:10px;box-shadow:0 4px 8px rgba(0,0,0,0.1);}");
              client.println("h1,h2{color:#2e7d32;text-align:center;}");
              client.println(".sensor-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:15px;margin-bottom:20px;}");
              client.println(".sensor-item{background-color:#f1f8e9;padding:15px;border-radius:8px;border:1px solid #c8e6c9;}");
              client.println(".sensor-item p{margin:5px 0;font-size:1em;} .sensor-item span{font-weight:bold;color:#388e3c;}");
              client.println(".controls{text-align:center;margin-bottom:20px;}");
              client.println(".controls a, .controls button{display:inline-block;background-color:#4caf50;color:white;padding:12px 22px;margin:8px;text-decoration:none;border-radius:25px;font-size:1em;border:none;cursor:pointer;transition:background-color 0.3s;}");
              client.println(".controls a:hover, .controls button:hover{background-color:#388e3c;}");
              client.println(".status-leds{text-align:center;margin-bottom:20px;} .led{display:inline-block;width:20px;height:20px;border-radius:50%;margin:0 10px;background-color:#ccc;border:1px solid #999;}");
              client.println(".led.on{background-color:lime;box-shadow:0 0 10px lime;} .led.warn{background-color:orange;box-shadow:0 0 10px orange;}");
              client.println("footer{text-align:center;margin-top:20px;font-size:0.9em;color:#555;}");
              client.println("</style></head><body><div class=\"container\">");
              client.println("<h1>&#128663; Ciftci Dostu Kontrol Paneli v2 &#127806;</h1>");

              // Durum LED'leri (Simülasyon)
              client.println("<h2>Durum Gostergeleri</h2><div class=\"status-leds\">");
              client.print("WiFi: <span class=\"led " + String(WiFi.status() == WL_CONNECTED ? "on" : "") + "\"></span>");
              client.print("Hareket: <span class=\"led " + String(digitalRead(LED_MOVEMENT_PIN) ? "on" : "") + "\"></span>");
              client.print("Dusuk Batarya: <span class=\"led " + String(lowBatteryAlarmActive ? "warn" : "") + "\"></span>");
              client.println("</div>");

              // Sensör Verileri
              client.println("<h2>Sensör Verileri</h2><div class=\"sensor-grid\">");
              // Mesafe
              float distance = readDistance();
              client.println("<div class=\"sensor-item\"><p>Mesafe: <span>" + String(distance) + " cm</span></p></div>");
              // Sıcaklık & Nem
              float temp = dht.readTemperature();
              float hum = dht.readHumidity();
              client.println("<div class=\"sensor-item\"><p>Ortam Sicakligi: <span>" + String(temp) + " &deg;C</span></p>");
              client.println("<p>Ortam Nemi: <span>" + String(hum) + " %</span></p></div>");
              // Batarya
              client.println("<div class=\"sensor-item\"><p>Batarya Voltajı: <span>" + String(batteryVoltage, 2) + " V</span></p></div>");
              // GPS
              client.print("<div class=\"sensor-item\"><p>GPS Konumu: <span>");
              if (gps.location.isValid())
              {
                client.print("Lat: " + String(gps.location.lat(), 6) + ", Lng: " + String(gps.location.lng(), 6));
              }
              else
              {
                client.print("Veri Alinamiyor");
              }
              client.println("</span></p></div>");
              // MPU6050
              client.println("<div class=\"sensor-item\"><p>MPU6050 Acc (X,Y,Z): <span>" + String(mpu_accX, 1) + ", " + String(mpu_accY, 1) + ", " + String(mpu_accZ, 1) + " m/s&sup2;</span></p>");
              client.println("<p>MPU6050 Gyro (X,Y,Z): <span>" + String(mpu_gyroX, 1) + ", " + String(mpu_gyroY, 1) + ", " + String(mpu_gyroZ, 1) + " rad/s</span></p>");
              client.println("<p>MPU6050 Sicaklik: <span>" + String(mpu_temp, 1) + " &deg;C</span></p></div>");
              // Toprak Nemi
              client.println("<div class=\"sensor-item\"><p>Toprak Nemi (Analog): <span>" + String(soilMoistureValue) + "</span></p></div>");
              client.println("</div>"); // sensor-grid sonu

              // Motor Kontrolleri
              client.println("<h2>Motor Kontrol</h2><div class=\"controls\">");
              client.println("<a href=\"/forward\">&#9650; Ileri</a>");
              client.println("<a href=\"/left\">&#9664; Sol</a>");
              client.println("<a href=\"/stop\">&#9209; Dur</a>");
              client.println("<a href=\"/right\">&#9654; Sag</a>");
              client.println("<a href=\"/back\">&#9660; Geri</a>");
              // Hız ayarı için input eklenebilir:
              // client.println("<p>Hiz (0-255): <input type='number' id='speedVal' value='" + String(motorSpeed) + "' min='0' max='255' onchange='setSpeed()'></p>");
              // client.println("<script>function setSpeed(){ var speed = document.getElementById('speedVal').value; window.location.href = '/speed?val=' + speed;}</script>");
              client.println("</div>");

              client.println("</div><footer><p>&copy; 2024-2025 Ciftci Dostu Projesi</p></footer></body></html>");
              client.println();
              break;
            }
            else
            {
              currentLine = "";
            }
          }
          else if (c != '\r')
          {
            currentLine += c;
          }

          // İstek yolunu yakala (sadece ilk satırda)
          if (requestPath == "" && currentLine.startsWith("GET "))
          {
            int firstSpace = currentLine.indexOf(' ');
            int secondSpace = currentLine.indexOf(' ', firstSpace + 1);
            if (secondSpace > firstSpace)
            {
              requestPath = currentLine.substring(firstSpace + 1, secondSpace);
            }
          }
        }
      }
      client.stop();
      Serial.println("Web Istemci Baglantisi Kapatildi.");
    }
  }
  delay(10);
}

