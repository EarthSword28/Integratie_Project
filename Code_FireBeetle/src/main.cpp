// BRONNEN
  // Jorden: ChatGPT voor te kijken hoe ik de seriële monitor kon gebruiken met een Firebeetle ESP32-S3: https://chatgpt.com/share/68221f8f-84dc-800c-90e9-fb83332ecd26 (12/05/2025)
  // Jorden: Voorbeeld code voor de SHT45: https://learn.adafruit.com/adafruit-sht40-temperature-humidity-sensor/arduino (12/05/2025)
  // Jorden: Meerder instanties van een DHT22 uitlezen: https://forum.arduino.cc/t/getting-multiple-readings-of-dht22-sensors-using-esp32/1020613 (12/05/2025)
  // Jorden: ChatGPT voor meerder instanties van een SHT45 uit te lezen: https://chatgpt.com/share/6835b6cf-d988-800c-b2c6-ab10d9c639a6 (27/05/2025)

// INCLUDES:
#include <Arduino.h>

  // de extra files die deel maken van de code en die ij zelf geschreven hebben
#include <G7_config.h>

  // de gebruikte libraries
#include <Adafruit_SHT4x.h>
#include <DHTesp.h>

// de gebruikte pinnen
#define G7_sensorOutside 13

#define G7_sda1 1
#define G7_scl1 2

#define G7_sda2 10
#define G7_scl2 11

// activatie/configuratie sensoren
  // Maak een nieuwe I²C-bus aan op andere pinnen
TwoWire I2C_1 = TwoWire(0);  // je kunt 0 of 1 gebruiken
TwoWire I2C_2 = TwoWire(1);

Adafruit_SHT4x sht1 = Adafruit_SHT4x();
Adafruit_SHT4x sht2 = Adafruit_SHT4x();

DHTesp dht22;

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


// functie om de precisie van de eerste SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT4xSetupPrecision1() {
  // You can have 3 different precisions, higher precision takes longer (from SHT45 Example Code)
  sht1.setPrecision(SHT4X_HIGH_PRECISION);
  if (sht1.getPrecision() == SHT4X_HIGH_PRECISION) {
    Serial.println("High precision");
  }
  else if (sht1.getPrecision() == SHT4X_MED_PRECISION) {
    Serial.println("Med precision");
  }
  else if (sht1.getPrecision() == SHT4X_LOW_PRECISION) {
    Serial.println("Low precision");
  }
}

// functie om de heater van de eerste SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT4xSetupHeater1() {
  // You can have 6 different heater settings     (from SHT45 Example Code)
  // higher heat and longer times uses more power (from SHT45 Example Code)
  // and reads will take longer too!              (from SHT45 Example Code)
  sht1.setHeater(SHT4X_NO_HEATER);
  if (sht1.getHeater() == SHT4X_NO_HEATER) {
    Serial.println("No heater");
  }
  else if (sht1.getHeater() == SHT4X_HIGH_HEATER_1S) {
    Serial.println("High heat for 1 second");
  }
  else if (sht1.getHeater() == SHT4X_HIGH_HEATER_100MS) {
    Serial.println("High heat for 0.1 second");
  }
  else if (sht1.getHeater() == SHT4X_MED_HEATER_1S) {
    Serial.println("Medium heat for 1 second");
  }
  else if (sht1.getHeater() == SHT4X_MED_HEATER_100MS) {
    Serial.println("Medium heat for 0.1 second");
  }
  else if (sht1.getHeater() == SHT4X_LOW_HEATER_1S) {
    Serial.println("Low heat for 1 second");
  }
  else if (sht1.getHeater() == SHT4X_LOW_HEATER_100MS) {
    Serial.println("Low heat for 0.1 second");
  }
}

// functie om de eerste SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT45Setup1() {
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens (from SHT45 Example Code)
  
  G7_sensorenIntervalTimer = millis();

  if (! sht1.begin(&I2C_1)) {
    Serial.println("Couldn't find the first SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found the first SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht1.readSerial(), HEX);
  
  G7_SHT4xSetupPrecision1();
  G7_SHT4xSetupHeater1();
}

// functie om de precisie van de tweede SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT4xSetupPrecision2() {
  // You can have 3 different precisions, higher precision takes longer (from SHT45 Example Code)
  sht2.setPrecision(SHT4X_HIGH_PRECISION);
  if (sht2.getPrecision() == SHT4X_HIGH_PRECISION) {
    Serial.println("High precision");
  }
  else if (sht2.getPrecision() == SHT4X_MED_PRECISION) {
    Serial.println("Med precision");
  }
  else if (sht2.getPrecision() == SHT4X_LOW_PRECISION) {
    Serial.println("Low precision");
  }
}

// functie om de heater van de tweede SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT4xSetupHeater2() {
  // You can have 6 different heater settings     (from SHT45 Example Code)
  // higher heat and longer times uses more power (from SHT45 Example Code)
  // and reads will take longer too!              (from SHT45 Example Code)
  sht2.setHeater(SHT4X_NO_HEATER);
  if (sht2.getHeater() == SHT4X_NO_HEATER) {
    Serial.println("No heater");
  }
  else if (sht2.getHeater() == SHT4X_HIGH_HEATER_1S) {
    Serial.println("High heat for 1 second");
  }
  else if (sht2.getHeater() == SHT4X_HIGH_HEATER_100MS) {
    Serial.println("High heat for 0.1 second");
  }
  else if (sht2.getHeater() == SHT4X_MED_HEATER_1S) {
    Serial.println("Medium heat for 1 second");
  }
  else if (sht2.getHeater() == SHT4X_MED_HEATER_100MS) {
    Serial.println("Medium heat for 0.1 second");
  }
  else if (sht2.getHeater() == SHT4X_LOW_HEATER_1S) {
    Serial.println("Low heat for 1 second");
  }
  else if (sht2.getHeater() == SHT4X_LOW_HEATER_100MS) {
    Serial.println("Low heat for 0.1 second");
  }
}

// functie om de tweede SHT45 te configureren, deze code is direct afkomstig uit de voorbeeld code voor de SHT45
void G7_SHT45Setup2() {
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens (from SHT45 Example Code)
  
  G7_sensorenIntervalTimer = millis();

  if (! sht2.begin(&I2C_2)) {
    Serial.println("Couldn't find the second SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found the second SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht2.readSerial(), HEX);
  
  G7_SHT4xSetupPrecision2();
  G7_SHT4xSetupHeater2();
}

// functie om alle code die normaal in de void setup() functie goed te laten werken, ook als deze geïntegreerd word in het grotere geheel van de code waar ook de andere groepen aan gewerkt hebben
void G7_setup() {
  Serial.begin(9600);  

  I2C_1.begin(G7_sda1, G7_scl1);
  I2C_2.begin(G7_sda2, G7_scl2);

  G7_SHT45Setup1();
  G7_SHT45Setup2();

  dht22.setup(G7_sensorOutside, DHTesp::DHT22);
}

// functie om de temperatuur en de vochtigheid in de broedkamer op te halen
void G7_getDataCore() {
  sensors_event_t humidity, temp;
  sht1.getEvent(&humidity, &temp);  // populate temp and humidity objects with fresh data (from SHT45 Example Code)

  G7_humidityCore = humidity.relative_humidity;
  G7_temperatureCore = temp.temperature;
}

// functie om de temperatuur en de vochtigheid binnen de kast op te halen
void G7_getDataWall() {
  sensors_event_t humidity, temp;
  sht2.getEvent(&humidity, &temp);  // populate temp and humidity objects with fresh data (from SHT45 Example Code)

  G7_humidityWall = humidity.relative_humidity;
  G7_temperatureWall = temp.temperature;
}

// functie om de temperatuur en de vochtigheid buiten de kast op te halen
void G7_getDataOutside() {
  G7_temperatureOutside = dht22.getTemperature();
  G7_humidityOutside = dht22.getHumidity();
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