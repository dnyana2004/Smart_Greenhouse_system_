/****************************************************
   🌱 SMART GREENHOUSE SYSTEM – FINAL MERGED CODE
   Includes:
   ✔ Blynk Cloud Integration
   ✔ ThingSpeak Upload
   ✔ Google Sheets Upload
   ✔ DHT11 Temperature & Humidity
   ✔ Soil Moisture Sensor
   ✔ Crop Selection (Mushroom / Tomato)
   ✔ LED Control with PWM
****************************************************/

// -------------------- BLYNK -------------------------
#define BLYNK_TEMPLATE_ID   "TMPL3KM5X-TOz"
#define BLYNK_TEMPLATE_NAME "Smart GreenHouse Sytem"
#define BLYNK_AUTH_TOKEN    "zgMDvvIEFOdVqEGIk8RY6XMzBUusw4LC"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "ThingSpeak.h"
#include "DHT.h"

// -------------------- WIFI --------------------------
const char* ssid = "Abhiraj 2";
const char* password = "ABhi2#4884";

// -------------------- GOOGLE SHEETS ------------------
const char* GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbxSP80IMK3qxv54TAzZ0i52TfX0OgepgjSU-1P4hwcTg1xc6mt4vULp-i6m1qt4IVlj/exec";

// -------------------- THINGSPEAK ---------------------
unsigned long myChannelID = 3098674;
const char * myWriteAPIKey = "ZC96XOXK1C3I2NFB";
WiFiClient tsClient;

// -------------------- DHT11 SENSOR -------------------
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// -------------------- SOIL SENSOR --------------------
#define SOIL_DIGITAL 5   // D1 → GPIO5

// -------------------- LEDs ---------------------------
#define LED_TEMP D0
#define LED_HUM  D1
#define SOIL_LED D2

// -------------------- CROP SETTINGS -------------------
struct CropSettings {
  float tempMin, tempMax;
  float humMin, humMax;
};

CropSettings mushroom = {18, 24, 70, 90};
CropSettings tomato   = {20, 30, 50, 70};

int cropChoice = 0;   // 0 = none selected
BlynkTimer timer;

// -------------------- WIFI RECONNECT -----------------
void connectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
      delay(300);
      retry++;
    }
  }
}

// -------------------- SEND TO GOOGLE -----------------
void sendToGoogle(float t, float h, int soilV) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  http.begin(client, GOOGLE_SCRIPT_URL);
  http.addHeader("Content-Type", "application/json");

  String jsonData =
    "{\"temperature\":" + String(t) +
    ",\"humidity\":" + String(h) +
    ",\"soil\":" + String(soilV) + "}";

  http.POST(jsonData);
  http.end();
}

// -------------------- SENSOR FUNCTION ----------------
void readSensors() {

  if (cropChoice == 0) {
    Serial.println("⚠ Select Crop: 1 = Mushroom, 2 = Tomato");
    return;
  }

  CropSettings crop =
    (cropChoice == 1) ? mushroom : tomato;

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soilStatus = digitalRead(SOIL_DIGITAL);
  int soilValue = (soilStatus == HIGH) ? 0 : 1;

  if (isnan(h) || isnan(t)) {
    Serial.println("❌ DHT11 Error!");
    return;
  }

  // ---- Serial Clean Output ----
  Serial.print("🌡 Temp: "); Serial.print(t);
  Serial.print("°C | 💧 Hum: "); Serial.print(h);
  Serial.print("% | 🌱 Soil: ");
  Serial.println(soilValue == 1 ? "WET" : "DRY");

  // ---- LED Control ----
  digitalWrite(SOIL_LED, soilStatus == HIGH ? HIGH : LOW);

  // Temperature PWM
  int tempPWM = 0;
  if (t < crop.tempMin)
    tempPWM = map(crop.tempMin - t, 0, 10, 50, 255);
  else if (t > crop.tempMax)
    tempPWM = map(t - crop.tempMax, 0, 10, 50, 255);
  analogWrite(LED_TEMP, tempPWM);

  // Humidity PWM
  int humPWM = 0;
  if (h < crop.humMin)
    humPWM = map(crop.humMin - h, 0, 20, 50, 255);
  else if (h > crop.humMax)
    humPWM = map(h - crop.humMax, 0, 20, 50, 255);
  analogWrite(LED_HUM, humPWM);

  // ---- Blynk Update ----
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, soilValue);

  // ---- ThingSpeak ----
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, soilValue);
  ThingSpeak.writeFields(myChannelID, myWriteAPIKey);

  // ---- Google Sheets ----
  sendToGoogle(t, h, soilValue);
}

// -------------------- SETUP ---------------------------
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(SOIL_DIGITAL, INPUT);
  pinMode(LED_TEMP, OUTPUT);
  pinMode(LED_HUM, OUTPUT);
  pinMode(SOIL_LED, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(400);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  ThingSpeak.begin(tsClient);

  Serial.println("📢 Enter Crop Number: 1 = Mushroom | 2 = Tomato");

  timer.setInterval(5000L, readSensors);  // every 5 sec
}

// -------------------- LOOP ----------------------------
void loop() {
  Blynk.run();
  timer.run();
  connectWiFi();

  // crop selection
  if (Serial.available() > 0) {
    int user = Serial.parseInt();
    if (user == 1 || user == 2) {
      cropChoice = user;
      Serial.print("✔ Crop Selected: ");
      Serial.println(user == 1 ? "Mushroom" : "Tomato");
    }
  }
}


















// #include "DHT.h"
// #include <ESP8266WiFi.h>
// #include "ThingSpeak.h"

// // --- WiFi Credentials ---
// const char* ssid = "1x";
// const char* password = "shravani@123";

// // --- ThingSpeak ---
// unsigned long myChannelID = 3098674;
// const char * myWriteAPIKey = "ZC96XOXK1C3I2NFB";
// WiFiClient client;

// // --- DHT11 Sensor ---
// #define DHTPIN 2      // GPIO4 = D2 pin on NodeMCU
// #define DHTTYPE DHT11
// DHT dht(DHTPIN, DHTTYPE);

// // Soil Moisture Sensor
// #define SOIL_DIGITAL 5   // D1 = GPIO5

// // LEDs
// #define LED_TEMP D0
// #define LED_HUM  D1
// #define SOIL_LED D2

// int cropChoice = 0;

// // Crop ranges
// struct CropSettings {
//   float tempMin, tempMax;
//   float humMin, humMax;
// };

// CropSettings mushroom = {18, 24, 70, 90};
// CropSettings tomato   = {20, 30, 50, 70};

// void setup() {
//   Serial.begin(115200);
//   dht.begin();

//   pinMode(SOIL_DIGITAL, INPUT);
//   pinMode(SOIL_LED, OUTPUT);
//   pinMode(LED_TEMP, OUTPUT);
//   pinMode(LED_HUM, OUTPUT);

//   // WiFi Connect
//   WiFi.begin(ssid, password);
//   Serial.print("Connecting to WiFi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\n WiFi Connected!");
  
//   ThingSpeak.begin(client);

//   Serial.println("Smart Greenhouse System Started...");
//   Serial.println("Enter Crop Number: 1 = Mushroom | 2 = Tomato");
// }

// void loop() {
//   // Crop selection
//   if (Serial.available() > 0) {
//     int userInput = Serial.parseInt();
//     if (userInput == 1 || userInput == 2) {
//       cropChoice = userInput;
//       Serial.print("Crop Selected: ");
//       if (cropChoice == 1) Serial.println("Mushroom");
//       if (cropChoice == 2) Serial.println("Tomato");
//     }
//   }

//   if (cropChoice == 0) {
//     Serial.println("Select crop first (1=Mushroom, 2=Tomato)");
//     delay(2000);
//     return;
//   }

//   // Select crop settings
//   CropSettings crop;
//   switch (cropChoice) {
//     case 1: crop = mushroom; break;
//     case 2: crop = tomato;   break;
//   }

//   // Read sensors
//   float humidity = dht.readHumidity();
//   float temperature = dht.readTemperature();
//   int soilStatus = digitalRead(SOIL_DIGITAL);

//   if (isnan(humidity) || isnan(temperature)) {
//     Serial.println("Failed to read from DHT11 sensor!");
//     return;
//   }

//   Serial.print("🌡 Temp: "); Serial.print(temperature);
//   Serial.print(" °C | Humidity: "); Serial.print(humidity);
//   Serial.println(" %");

//   // Soil Moisture
//   int soilValue = (soilStatus == HIGH) ? 0 : 1; // 1=Wet, 0=Dry
//   if (soilStatus == HIGH) {
//     Serial.println("Soil is DRY → Needs Water!");
//     digitalWrite(SOIL_LED, HIGH);
//   } else {
//     Serial.println("Soil Moisture OK");
//     digitalWrite(SOIL_LED, LOW);
//   }

//   // Temperature Control
//   int tempPWM = 0;
//   if (temperature < crop.tempMin) {
//     tempPWM = map(crop.tempMin - temperature, 0, 10, 50, 255);
//     Serial.print("Heating (LED Brightness = "); Serial.print(tempPWM); Serial.println(")");
//   } else if (temperature > crop.tempMax) {
//     tempPWM = map(temperature - crop.tempMax, 0, 10, 50, 255);
//     Serial.print("Cooling (LED Brightness = "); Serial.print(tempPWM); Serial.println(")");
//   }
//   analogWrite(LED_TEMP, tempPWM);

//   // Humidity Control
//   int humPWM = 0;
//   if (humidity < crop.humMin) {
//     humPWM = map(crop.humMin - humidity, 0, 20, 50, 255);
//     Serial.print("Humidifying (LED Brightness = "); Serial.print(humPWM); Serial.println(")");
//   } else if (humidity > crop.humMax) {
//     humPWM = map(humidity - crop.humMax, 0, 20, 50, 255);
//     Serial.print("Dehumidifying (LED Brightness = "); Serial.print(humPWM); Serial.println(")");
//   }
//   analogWrite(LED_HUM, humPWM);

//   // --- Upload to ThingSpeak ---
//   ThingSpeak.setField(1, temperature);
//   ThingSpeak.setField(2, humidity);
//   ThingSpeak.setField(3, soilValue);

//   int statusCode = ThingSpeak.writeFields(myChannelID, myWriteAPIKey);
//   if (statusCode == 200) {
//     Serial.println("Data sent to ThingSpeak successfully!");
//   } else {
//     Serial.print("Problem sending data. Code: "); Serial.println(statusCode);
//   }

//   Serial.println("-----------------------------------");
//   delay(20000);
// }

feat: Added Smart Greenhouse System Arduino code

