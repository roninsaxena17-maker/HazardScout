
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <DHT.h>
#include <IBusBM.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// =====================================================
// HAZARDSCOUT - MAIN ESP32 DEVKIT V1
// Sensors + FlySky iBUS + BTS7960 + ThingSpeak
// =====================================================

// ---------------- Wi-Fi ----------------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---------------- ThingSpeak ----------------
const char* THINGSPEAK_WRITE_KEY = "YOUR_WRITE_API_KEY";
const char* THINGSPEAK_URL =
  "http://api.thingspeak.com/update";

// ---------------- Pin definitions ----------------
#define IBUS_RX_PIN 16

#define LEFT_RPWM  25
#define LEFT_LPWM  26
#define RIGHT_RPWM 27
#define RIGHT_LPWM 14

#define LEFT_REN  32
#define LEFT_LEN  33
#define RIGHT_REN 19
#define RIGHT_LEN 23

#define TRIG_PIN 5
#define ECHO_PIN 18

#define DHT_PIN 4
#define DHT_TYPE DHT11

#define MQ2_PIN 34

#define SDA_PIN 21
#define SCL_PIN 22

// ---------------- Objects ----------------
IBusBM ibus;
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_MPU6050 mpu;

// ---------------- Timing ----------------
const unsigned long DHT_INTERVAL = 2000;
const unsigned long SENSOR_INTERVAL = 250;
const unsigned long THINGSPEAK_INTERVAL = 15000;
const unsigned long RC_CHECK_INTERVAL = 50;

// ---------------- Sensor values ----------------
float distanceCm = -1;
float temperatureC = NAN;
float humidity = NAN;

int mq2Raw = 0;

float accelX = 0;
float accelY = 0;
float accelZ = 0;

float gyroX = 0;
float gyroY = 0;
float gyroZ = 0;

// ---------------- Receiver values ----------------
int throttleValue = 1500;
int steeringValue = 1500;

bool receiverValid = false;
unsigned long lastValidReceiverTime = 0;

// ---------------- Timing trackers ----------------
unsigned long lastDHTRead = 0;
unsigned long lastSensorRead = 0;
unsigned long lastThingSpeakUpload = 0;
unsigned long lastRCCheck = 0;

// ---------------- Motor settings ----------------
const int MOTOR_MAX_PWM = 200;
const int MOTOR_DEADZONE = 40;


// =====================================================
// MOTOR CONTROL
// =====================================================

void enableMotorDrivers() {
  digitalWrite(LEFT_REN, HIGH);
  digitalWrite(LEFT_LEN, HIGH);

  digitalWrite(RIGHT_REN, HIGH);
  digitalWrite(RIGHT_LEN, HIGH);
}

void stopMotors() {
  analogWrite(LEFT_RPWM, 0);
  analogWrite(LEFT_LPWM, 0);

  analogWrite(RIGHT_RPWM, 0);
  analogWrite(RIGHT_LPWM, 0);
}

void setMotor(int rpwmPin, int lpwmPin, int speedValue) {
  speedValue = constrain(speedValue, -255, 255);

  if (abs(speedValue) < MOTOR_DEADZONE) {
    analogWrite(rpwmPin, 0);
    analogWrite(lpwmPin, 0);
    return;
  }

  if (speedValue > 0) {
    analogWrite(rpwmPin, speedValue);
    analogWrite(lpwmPin, 0);
  } else {
    analogWrite(rpwmPin, 0);
    analogWrite(lpwmPin, abs(speedValue));
  }
}

void driveMotors(int leftSpeed, int rightSpeed) {
  leftSpeed = constrain(leftSpeed, -MOTOR_MAX_PWM, MOTOR_MAX_PWM);
  rightSpeed = constrain(rightSpeed, -MOTOR_MAX_PWM, MOTOR_MAX_PWM);

  setMotor(LEFT_RPWM, LEFT_LPWM, leftSpeed);
  setMotor(RIGHT_RPWM, RIGHT_LPWM, rightSpeed);
}


// =====================================================
// FLYSKY iBUS RECEIVER
// =====================================================

int readIBusChannel(uint8_t channel) {
  return ibus.readChannel(channel);
}

bool channelLooksValid(int value) {
  return value >= 900 && value <= 2100;
}

void updateReceiver() {
  ibus.loop();

  int newThrottle = readIBusChannel(1);
  int newSteering = readIBusChannel(0);

  if (channelLooksValid(newThrottle) &&
      channelLooksValid(newSteering)) {

    throttleValue = newThrottle;
    steeringValue = newSteering;

    receiverValid = true;
    lastValidReceiverTime = millis();
  }

  if (!receiverValid ||
      millis() - lastValidReceiverTime > 500) {

    receiverValid = false;
    throttleValue = 1500;
    steeringValue = 1500;

    stopMotors();
  }
}

int mapReceiverToMotor(int receiverValue) {
  int motorValue = map(receiverValue, 1000, 2000, -255, 255);
  return constrain(motorValue, -255, 255);
}

void processDriveControls() {
  if (!receiverValid) {
    stopMotors();
    return;
  }

  int throttle = mapReceiverToMotor(throttleValue);
  int steering = mapReceiverToMotor(steeringValue);

  int leftSpeed = throttle + steering;
  int rightSpeed = throttle - steering;

  int largest = max(abs(leftSpeed), abs(rightSpeed));

  if (largest > MOTOR_MAX_PWM) {
    leftSpeed = leftSpeed * MOTOR_MAX_PWM / largest;
    rightSpeed = rightSpeed * MOTOR_MAX_PWM / largest;
  }

  driveMotors(leftSpeed, rightSpeed);
}


// =====================================================
// HC-SR04 ULTRASONIC SENSOR
// =====================================================

float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
    pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2.0;
}


// =====================================================
// DHT11
// =====================================================

void readDHT() {
  if (millis() - lastDHTRead < DHT_INTERVAL) {
    return;
  }

  lastDHTRead = millis();

  float newTemperature = dht.readTemperature();
  float newHumidity = dht.readHumidity();

  if (!isnan(newTemperature)) {
    temperatureC = newTemperature;
  }

  if (!isnan(newHumidity)) {
    humidity = newHumidity;
  }
}


// =====================================================
// MPU-6050
// =====================================================

void readMPU6050() {
  sensors_event_t acceleration;
  sensors_event_t gyro;
  sensors_event_t temperature;

  mpu.getEvent(
    &acceleration,
    &gyro,
    &temperature
  );

  accelX = acceleration.acceleration.x;
  accelY = acceleration.acceleration.y;
  accelZ = acceleration.acceleration.z;

  gyroX = gyro.gyro.x;
  gyroY = gyro.gyro.y;
  gyroZ = gyro.gyro.z;
}


// =====================================================
// SENSOR COLLECTION
// =====================================================

void readSensors() {
  if (millis() - lastSensorRead < SENSOR_INTERVAL) {
    return;
  }

  lastSensorRead = millis();

  distanceCm = readDistance();
  mq2Raw = analogRead(MQ2_PIN);

  readDHT();
  readMPU6050();
}


// =====================================================
// THINGSPEAK
// =====================================================

void uploadToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (millis() - lastThingSpeakUpload <
      THINGSPEAK_INTERVAL) {
    return;
  }

  lastThingSpeakUpload = millis();

  HTTPClient http;

  String url = String(THINGSPEAK_URL) +
    "?api_key=" + THINGSPEAK_WRITE_KEY +
    "&field1=" + String(distanceCm, 2) +
    "&field2=" + String(temperatureC, 2) +
    "&field3=" + String(humidity, 2) +
    "&field4=" + String(mq2Raw);

  http.begin(url);

  int responseCode = http.GET();

  Serial.print("ThingSpeak response: ");
  Serial.println(responseCode);

  http.end();
}


// =====================================================
// WIFI
// =====================================================

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 15000) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection failed");
  }
}


// =====================================================
// SERIAL MONITOR
// =====================================================

void printSensorData() {
  Serial.println("------ HAZARDSCOUT ------");

  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("MQ-2 raw: ");
  Serial.println(mq2Raw);

  Serial.print("Accel X: ");
  Serial.println(accelX);

  Serial.print("Accel Y: ");
  Serial.println(accelY);

  Serial.print("Accel Z: ");
  Serial.println(accelZ);

  Serial.print("Receiver valid: ");
  Serial.println(receiverValid ? "YES" : "NO");

  Serial.print("Throttle: ");
  Serial.println(throttleValue);

  Serial.print("Steering: ");
  Serial.println(steeringValue);

  Serial.println("-------------------------");
}


// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_RPWM, OUTPUT);
  pinMode(LEFT_LPWM, OUTPUT);
  pinMode(RIGHT_RPWM, OUTPUT);
  pinMode(RIGHT_LPWM, OUTPUT);

  pinMode(LEFT_REN, OUTPUT);
  pinMode(LEFT_LEN, OUTPUT);
  pinMode(RIGHT_REN, OUTPUT);
  pinMode(RIGHT_LEN, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(MQ2_PIN, INPUT);

  stopMotors();
  enableMotorDrivers();

  dht.begin();

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not detected");
  } else {
    Serial.println("MPU6050 connected");

    mpu.setAccelerometerRange(
      MPU6050_RANGE_8_G
    );

    mpu.setGyroRange(
      MPU6050_RANGE_500_DEG
    );

    mpu.setFilterBandwidth(
      MPU6050_BAND_21_HZ
    );
  }

  Serial2.begin(
    115200,
    SERIAL_8N1,
    IBUS_RX_PIN,
    -1
  );

  ibus.begin(Serial2);

  connectWiFi();

  Serial.println("HazardScout firmware started");
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {
  updateReceiver();

  if (millis() - lastRCCheck >= RC_CHECK_INTERVAL) {
    lastRCCheck = millis();
    processDriveControls();
  }

  readSensors();

  uploadToThingSpeak();

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    printSensorData();
  }
}
