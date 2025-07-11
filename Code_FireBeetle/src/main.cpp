#include <Arduino.h>
#include "config.h"           // Eigen configuratiebestand
#include <WiFi.h>             // Standaard ESP32 WiFi
#include <WiFiClientSecure.h> // Voor HTTPS
#include <HTTPClient.h>       // Voor HTTP requests
#include <ArduinoJson.h>      // Voor JSON manipulatie
#include <Wire.h>             // Voor I2C
#include <Adafruit_Sensor.h>  // Basis voor Adafruit sensoren
#include <DHT.h>              // Voor DHT sensoren
#include <DHT_U.h>            // Unified interface voor DHT
#include <SensirionI2cSht4x.h> // De bedoelde SHT4x bibliotheek
#include <SensirionCore.h>    // Nodig voor errorToString
#include <NTPClient.h>        // Voor tijd synchronisatie
#include <WiFiUdp.h>          // Voor NTP
#include <time.h>             // Voor tijd formattering (strftime)
#include <esp_sleep.h>        // Voor deep sleep functionaliteit

// --- Sensor Pin & Type Instellingen (nu in main.cpp) ---
#define DHT1_PIN 13
#define DHT2_PIN 14
#define DHT_TYPE DHT22

// --- Sensor IDs ---
const int SENSOR_ID_DHT1_TEMP = 1;
const int SENSOR_ID_DHT1_HUMID = 2;
const int SENSOR_ID_DHT2_TEMP = 3;
const int SENSOR_ID_DHT2_HUMID = 4;
const int SENSOR_ID_SHT_TEMP = 5;
const int SENSOR_ID_SHT_HUMID = 6;
const int SENSOR_ID_HX711_WEIGHT = 7;

// --- Globale Objecten ---
DHT_Unified dht1(DHT1_PIN, DHT_TYPE);
DHT_Unified dht2(DHT2_PIN, DHT_TYPE);
SensirionI2cSht4x sht4x;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, ntp_update_interval_ms);

WiFiClientSecure secureClient;
HTTPClient http;

// Functie declaraties
void connectWiFi();
void syncNTPTime();
String getFormattedTimestamp();
void sendMeasurement(int sensorId, const String& timestamp, const char* valueKey, float value);
void print_wakeup_reason();

void setup() {
    // Optionele delay voor USB enumeratie, vooral bij native USB
    // Probeer waarden tussen 500-2000 ms of verwijder als het niet helpt.
    delay(1000);

    Serial.begin(115200);

    // Wacht tot de Serial Monitor verbonden is, met een timeout.
    unsigned long serial_connect_start_time = millis();
    // Wacht maximaal 5 seconden (5000 ms) op de Serial Monitor.
    while (!Serial && (millis() - serial_connect_start_time < 5000)) {
        delay(100); // Korte pauze om de CPU niet te overbelasten
    }
    // Als Serial nog steeds niet beschikbaar is na de timeout, gaat de code gewoon verder.
    // Dit voorkomt dat het programma vastloopt als er geen Serial Monitor is aangesloten.

    Serial.println("\nESP32 Sensor Data Logger - Deepsleep (SHT4x 0x44, Serial Wait)");

    print_wakeup_reason();

    Wire.begin();
    dht1.begin();
    dht2.begin();
    sht4x.begin(Wire, 0x44);
    Serial.println("SHT4x: Attempted to initialize with address 0x44.");

    connectWiFi();

    if (WiFi.status() == WL_CONNECTED) {
        secureClient.setInsecure();
        syncNTPTime();
        String currentTimestamp = getFormattedTimestamp();

        if (currentTimestamp != "TIME_NOT_SET") {
            Serial.println("Current Timestamp: " + currentTimestamp);
            // ... (rest van je sensor code) ...
             sensors_event_t event_dht;
            float temp_val, hum_val, weight_val;

            // --- DHT1 (PIN13) ---
            temp_val = NAN; hum_val = NAN;
            dht1.temperature().getEvent(&event_dht);
            if (!isnan(event_dht.temperature)) temp_val = event_dht.temperature;
            sendMeasurement(SENSOR_ID_DHT1_TEMP, currentTimestamp, "temperature", temp_val);

            dht1.humidity().getEvent(&event_dht);
            if (!isnan(event_dht.relative_humidity)) hum_val = event_dht.relative_humidity;
            sendMeasurement(SENSOR_ID_DHT1_HUMID, currentTimestamp, "humidity", hum_val);

            // --- DHT2 (PIN14) ---
            temp_val = NAN; hum_val = NAN;
            dht2.temperature().getEvent(&event_dht);
            if (!isnan(event_dht.temperature)) temp_val = event_dht.temperature;
            sendMeasurement(SENSOR_ID_DHT2_TEMP, currentTimestamp, "temperature", temp_val);

            dht2.humidity().getEvent(&event_dht);
            if (!isnan(event_dht.relative_humidity)) hum_val = event_dht.relative_humidity;
            sendMeasurement(SENSOR_ID_DHT2_HUMID, currentTimestamp, "humidity", hum_val);

            // --- SHT4x (I2C) ---
            temp_val = NAN; hum_val = NAN;
            uint16_t errorSht;
            char errorMessageSht[256];
            errorSht = sht4x.measureHighPrecision(temp_val, hum_val);
            if (errorSht) {
                Serial.print("Error SHT4x measureHighPrecision: ");
                errorToString(errorSht, errorMessageSht, sizeof(errorMessageSht));
                Serial.println(errorMessageSht);
                temp_val = NAN; hum_val = NAN;
            }
            sendMeasurement(SENSOR_ID_SHT_TEMP, currentTimestamp, "temperature", temp_val);
            sendMeasurement(SENSOR_ID_SHT_HUMID, currentTimestamp, "humidity", hum_val);

            // --- HX711 (Mock Data) ---
            weight_val = random(5000, 350000) / 10.0;
            sendMeasurement(SENSOR_ID_HX711_WEIGHT, currentTimestamp, "weight", weight_val);

        } else {
            Serial.println("Time not set, cannot send measurements this cycle.");
        }
    } else {
        Serial.println("WiFi connection failed. Skipping data send this cycle.");
    }

    Serial.println("------------------------------------");
    Serial.println("Going to sleep for " + String(TIME_TO_SLEEP_SECONDS) + " seconds.");
    Serial.flush(); // Zorg dat alle seriële output verstuurd is

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_SECONDS * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}


void loop() {
    // Leeg
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void connectWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("ESP32 IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi.");
    }
}

void syncNTPTime() {
    Serial.println("Synchronizing time with NTP server...");
    timeClient.begin();
    if (timeClient.forceUpdate()) {
        time_t epochTime = timeClient.getEpochTime();
        struct timeval tv;
        tv.tv_sec = epochTime;
        tv.tv_usec = 0;
        settimeofday(&tv, nullptr);
        Serial.println("System time set from NTP.");
    } else {
        Serial.println("Failed to synchronize time with NTP server.");
    }
}

String getFormattedTimestamp() {
    time_t now;
    struct tm timeinfo;
    char buffer[20];

    time(&now);

    if ((now - gmtOffset_sec) < 1577836800L) {
        Serial.println("System time appears invalid or not set (UTC equivalent < 1 Jan 2020).");
        return "TIME_NOT_SET";
    }

    gmtime_r(&now, &timeinfo);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}

void sendMeasurement(int sensorId, const String& timestamp, const char* valueKey, float value) {
    if (timestamp == "TIME_NOT_SET") {
        Serial.printf("Sensor ID %d: Cannot send data, time not set.\n", sensorId);
        return;
    }
    if (isnan(value)) {
        Serial.printf("Sensor ID %d (%s): Value is NAN, skipping send.\n", sensorId, valueKey);
        return;
    }

    StaticJsonDocument<256> doc;
    doc["sensor_id"] = sensorId;
    doc["timestamp"] = timestamp.c_str();

    char floatBuffer[12];
    dtostrf(value, 1, 2, floatBuffer);
    doc[valueKey] = floatBuffer;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    String serverUrl = String("https://") + api_host + ":" + String(api_port) + api_path_sensor_data;

    Serial.printf("Sending to %s for sensor_id %d:\n%s\n", serverUrl.c_str(), sensorId, jsonPayload.c_str());

    if (http.begin(secureClient, serverUrl)) {
        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-API-Key", api_key);

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String responsePayload = http.getString();
            Serial.println("Response: " + responsePayload);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
            Serial.printf("[HTTPS] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();
    } else {
        Serial.printf("[HTTPS] Unable to connect to %s\n", serverUrl.c_str());
    }
    delay(750);
}
