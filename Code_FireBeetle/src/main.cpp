// BRONNEN
  // Jorden: ChatGPT voor te kijken hoe ik de seriële monitor kon gebruiken met een Firebeetle ESP32-S3: https://chatgpt.com/share/68221f8f-84dc-800c-90e9-fb83332ecd26 (12/05/2025)
  // Jorden: Voorbeeld code voor de SHT45: https://learn.adafruit.com/adafruit-sht40-temperature-humidity-sensor/arduino (12/05/2025)

// INCLUDES:
#include <Arduino.h>

  // de extra files die deel maken van de code en die ij zelf geschreven hebben
#include <G7_config.h>

  // de gebruikte libraries
#include <Adafruit_SHT4x.h>
#include <DHTesp.h>

// de gebruikte pinnen
#define G7_sensorWall 14
#define G7_sensorOutside 13

// activatie/configuratie sensoren
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

DHTesp dht1;
DHTesp dht2;

// VARIABELEN
unsigned long G7_huidigeMillis = 0;
unsigned long G7_sensorenIntervalTimer = 0;

float G7_temperatureCore = 0.0;
float G7_humidityCore = 0.0;

float G7_temperatureWall = 0.0;
float G7_humidityWall = 0.0;

float G7_temperatureOutside = 0.0;
float G7_humidityOutside = 0.0;

unsigned long G7_massa = 0;

char DataValues[105];


// functie om de precisie van de SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT4xSetupPrecision() {
  // You can have 3 different precisions, higher precision takes longer (from SHT45 Example Code)
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

// functie om de heater van de SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT4xSetupHeater() {
  // You can have 6 different heater settings     (from SHT45 Example Code)
  // higher heat and longer times uses more power (from SHT45 Example Code)
  // and reads will take longer too!              (from SHT45 Example Code)
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

// functie om de SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT45Setup() {
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens (from SHT45 Example Code)
  
  G7_sensorenIntervalTimer = millis();

  Serial.println("Adafruit SHT4x test");
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);
}

// functie om alle code die normaal in de void setup() functie goed te laten werken, ook als deze geïntegreerd word in het grotere geheel van de code waar ook de andere groepen aan gewerkt hebben
void G7_setup() {
  Serial.begin(9600);  

  G7_SHT45Setup();
  G7_SHT4xSetupPrecision();
  G7_SHT4xSetupHeater();

  dht1.setup(G7_sensorWall, DHTesp::DHT22);
  dht2.setup(G7_sensorOutside, DHTesp::DHT22);
}

// functie om de temperatuur en de vochtigheid in de broedkamer op te halen
void G7_getDataCore() {
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data (from SHT45 Example Code)

  G7_humidityCore = humidity.relative_humidity;
  G7_temperatureCore = temp.temperature;
}

// functie om de temperatuur en de vochtigheid binnen de kast op te halen
void G7_getDataWall() {
  G7_temperatureWall = dht1.getTemperature();
  G7_humidityWall = dht1.getHumidity();
}

// functie om de temperatuur en de vochtigheid buiten de kast op te halen
void G7_getDataOutside() {
  G7_temperatureOutside = dht2.getTemperature();
  G7_humidityOutside = dht2.getHumidity();
}

// functie om het gewicht van de kast op te halen
void G7_getMass() {
  G7_massa = 0; // PLACEHOLDER bij gebrek aan gewichtsensoren
}

// functie om de opgehaalde data door te sturen naar het centraal systeem
void G7_sendData(float coreTemp, float coreHumid, float wallTemp, float wallHumid, float outsideTemp, float outsideHumid, int massa) {
  // legenda: START@TEMP_CORE$coreTemp&HUMIDITY_CORE$coreHumid&TEMP_WALL$wallTemp&HUMIDITY_WALL$wallHumid&TEMP_OUT$outsideTemp&HUMIDITY_OUT$outsideHumid&MASS$massa@END
  sprintf(DataValues, "START@TEMP_CORE$%.2f&HUMIDITY_CORE$%.2f&TEMP_WALL$%.2f&HUMIDITY_WALL$%.2f&TEMP_OUT$%.2f&HUMIDITY_OUT$%.2f&MASS$%d@END", coreTemp, coreHumid, wallTemp, wallHumid, outsideTemp, outsideHumid, massa);  // verzamel alle variabelen in een string
  
  // stuur de string met variabelen door naar Python
  Serial.println(DataValues);
}

// DEBUG: functie om de opgehaalde data te lezen zonder ze door te sturen
void G7_debugData(float coreTemp, float coreHumid, float wallTemp, float wallHumid, float outsideTemp, float outsideHumid, int massa) {
  Serial.print("Temperature core: ");
  Serial.print(coreTemp);
  Serial.println(" °C");
  Serial.print("Humidity core: ");
  Serial.print(coreHumid);
  Serial.println(" % rH");
  
  Serial.print("Temperature wall: ");
  Serial.print(wallTemp);
  Serial.println(" °C");
  Serial.print("Humidity wall: ");
  Serial.print(wallHumid);
  Serial.println(" % rH");
  
  Serial.print("Temperature outside: ");
  Serial.print(outsideTemp);
  Serial.println(" °C");
  Serial.print("Humidity outside: ");
  Serial.print(outsideHumid);
  Serial.println(" % rH");

  Serial.print("Gewicht kast: ");
  Serial.print(massa);
  Serial.println(" Kg");

  Serial.println("----------");
}

// functie om alle code die normaal in de void loop() functie goed te laten werken, ook als deze geïntegreerd word in het grotere geheel van de code waar ook de andere groepen aan gewerkt hebben
void G7_loop() {
  G7_huidigeMillis = millis();

  if (G7_huidigeMillis >= G7_sensorenIntervalTimer) {
    G7_sensorenIntervalTimer = G7_huidigeMillis + G7_sensorenTijdInterval;

    // DONE: Haal data op (vochtigheid en temperatuur in core)
    G7_getDataCore();

    // DONE: Haal data op (vochtigheid en temperatuur in kast)
    G7_getDataWall();

    // DONE: Haal data op (vochtigheid en temperatuur buiten kast)
    G7_getDataOutside();

    // TODO: Haal data op (gewicht)
    G7_getMass();   // temporary

    // TODO: Stuur data door
    G7_sendData(G7_temperatureCore, G7_humidityCore, G7_temperatureWall, G7_humidityWall, G7_temperatureOutside, G7_humidityOutside, G7_massa);
    if (G7_DEBUG == HIGH) {
      G7_debugData(G7_temperatureCore, G7_humidityCore, G7_temperatureWall, G7_humidityWall, G7_temperatureOutside, G7_humidityOutside, G7_massa);
    }
  }
}

void setup() {
  G7_setup();
}

void loop() {
  G7_loop();
} 