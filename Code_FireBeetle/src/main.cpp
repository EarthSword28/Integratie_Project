// BRONNEN
  // Jorden: ChatGPT voor te kijken hoe ik de seriële monitor kon gebruiken met een Firebeetle ESP32-S3: https://chatgpt.com/share/68221f8f-84dc-800c-90e9-fb83332ecd26 (12/05/2025)
  // 

#include <Arduino.h>

#include <G7_config.h>

#include <Adafruit_SHT4x.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

#include <DHTesp.h>

#define G7_sensorWall 14
#define G7_sensorOutside 13

DHTesp dht1;
DHTesp dht2;

unsigned long G7_huidigeMillis = 0;
unsigned long G7_sensorenIntervalTimer = 0;

float G7_temperatureCore = 0.0;
float G7_humidityCore = 0.0;

float G7_temperatureWall = 0.0;
float G7_humidityWall = 0.0;

float G7_temperatureOutside = 0.0;
float G7_humidityOutside = 0.0;

char DataValues[90];

void G7_SHT4xSetupPrecision() {
  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  if (sht4.getPrecision() == SHT4X_HIGH_PRECISION) {
    Serial.println("High precision");
  }
  else if (sht4.getPrecision() == SHT4X_MED_PRECISION) {
    Serial.println("Med precision");
  }
  else if (sht4.getPrecision() == SHT4X_LOW_PRECISION) {
    Serial.println("Low precision");
  }
}

void G7_SHT4xSetupHeater() {
  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater(SHT4X_NO_HEATER);
  if (sht4.getHeater() == SHT4X_NO_HEATER) {
    Serial.println("No heater");
  }
  else if (sht4.getHeater() == SHT4X_HIGH_HEATER_1S) {
    Serial.println("High heat for 1 second");
  }
  else if (sht4.getHeater() == SHT4X_HIGH_HEATER_100MS) {
    Serial.println("High heat for 0.1 second");
  }
  else if (sht4.getHeater() == SHT4X_MED_HEATER_1S) {
    Serial.println("Medium heat for 1 second");
  }
  else if (sht4.getHeater() == SHT4X_MED_HEATER_100MS) {
    Serial.println("Medium heat for 0.1 second");
  }
  else if (sht4.getHeater() == SHT4X_LOW_HEATER_1S) {
    Serial.println("Low heat for 1 second");
  }
  else if (sht4.getHeater() == SHT4X_LOW_HEATER_100MS) {
    Serial.println("Low heat for 0.1 second");
  }
}

void G7_setup() {
  Serial.begin(9600);
  
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  
  G7_sensorenIntervalTimer = millis();

  Serial.println("Adafruit SHT4x test");
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  G7_SHT4xSetupPrecision();
  G7_SHT4xSetupHeater();

  dht1.setup(G7_sensorWall, DHTesp::DHT22);
  dht2.setup(G7_sensorOutside, DHTesp::DHT22);
}

// haal de temperatuur en de vochtigheid in de broedkamer op
void G7_getDataCore() {
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

  G7_humidityCore = humidity.relative_humidity;
  G7_temperatureCore = temp.temperature;
}

void G7_getDataWall() {
  G7_temperatureWall = dht1.getTemperature();
  G7_humidityWall = dht1.getHumidity();
}

void G7_getDataOutside() {
  G7_temperatureOutside = dht2.getTemperature();
  G7_humidityOutside = dht2.getHumidity();
}

void G7_sendData(float coreTemp, float coreHumid, float wallTemp, float wallHumid, float outsideTemp, float outsideHumid) {
  // Serial.print("Temperature core: ");
  // Serial.print(coreTemp);
  // Serial.println(" °C");
  // Serial.print("Humidity core: ");
  // Serial.print(coreHumid);
  // Serial.println(" % rH");
  
  // Serial.print("Temperature wall: ");
  // Serial.print(wallTemp);
  // Serial.println(" °C");
  // Serial.print("Humidity wall: ");
  // Serial.print(wallHumid);
  // Serial.println(" % rH");
  
  // Serial.print("Temperature outside: ");
  // Serial.print(outsideTemp);
  // Serial.println(" °C");
  // Serial.print("Humidity outside: ");
  // Serial.print(outsideHumid);
  // Serial.println(" % rH");

  // Serial.println("----------");

  // legenda: START@TEMP_CORE$HUMIDITY_CORE$TEMP_WALL$HUMIDITY_WALL$TEMP_OUT$HUMIDITY_OUT@coreTemp$coreHumid$wallTemp$wallHumid$outsideTemp$outsideHumid@END
  sprintf(DataValues, "START@TEMP_CORE$HUMIDITY_CORE$TEMP_WALL$HUMIDITY_WALL$TEMP_OUT$HUMIDITY_OUT@%.2f%.2f%.2f%.2f%.2f%.2f@END", coreTemp, coreHumid, wallTemp, wallHumid, outsideTemp, outsideHumid);  // verzamel alle variabelen in een string
  Serial.println(DataValues);  // stuur de string met variabelen door naar Python
}

void G7_loop() {
  G7_huidigeMillis = millis();

  if (G7_huidigeMillis >= G7_sensorenIntervalTimer) {
    G7_sensorenIntervalTimer = G7_huidigeMillis + G7_sensorenTijdInterval;

    // TODO: Haal data op (vochtigheid en temperatuur in core)
    G7_getDataCore();

    // TODO: Haal data op (vochtigheid en temperatuur in kast)
    G7_getDataWall();

    // TODO: Haal data op (vochtigheid en temperatuur buiten kast)
    G7_getDataOutside();

    // TODO: Haal data op (gewicht)

    // TODO: Stuur data door
    G7_sendData(G7_temperatureCore, G7_humidityCore, G7_temperatureWall, G7_humidityWall, G7_temperatureOutside, G7_humidityOutside);
  }
}

void setup() {
  G7_setup();
}

void loop() {
  G7_loop();
} 