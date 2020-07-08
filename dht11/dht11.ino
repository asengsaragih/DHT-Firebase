#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <time.h>

int timezone = 7 * 3600;
int dst = 0;

#define DHTPIN 2
#define DHTTYPE DHT11

#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "https://pets-firebase-starter-co-93c69.firebaseio.com/"
#define FIREBASE_AUTH "wbVDXk8ayCn00erGUbQ0L7MtJu2CFbCshANbc2C4"
#define WIFI_SSID "Biji kuda"
#define WIFI_PASSWORD "ganteng00"

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseJson json;

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

String currentTime;

void setup() {
  Serial.begin(115200);
  
  // Initialize device.
  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 10000;

  // wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  Serial.println("\nWaiting for Internet time");

  while(!time(nullptr)){
     Serial.print("*");
     delay(1000);
  }
  Serial.println("\nTime response....OK"); 
}

void loop() {
  // clear json before taking again
  
  // Delay between measurements.
  delay(30000);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);

  double hum, tem;
  
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    tem = event.temperature;
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    hum = event.relative_humidity;
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }

  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);

  int days = p_tm->tm_mday;
  int months = p_tm->tm_mon + 1;
  int years = p_tm->tm_year + 1900;
  int hours = p_tm->tm_hour + 2;
  int minutes = p_tm->tm_min;
  int seconds = p_tm->tm_sec;

  currentTime = String(days) + "/" + String(months) + "/" + String(years) + " " + String(hours) + ":" + String(minutes) + ":" + String(seconds);

  Serial.println(currentTime);
  
  if (WiFi.status() == WL_CONNECTED) {
    json.add("humidity", hum);
    json.add("temperature", tem);
    json.add("date", currentTime);
    Firebase.pushJSON(firebaseData, "Data/", json);
  }

  json.clear();
}
