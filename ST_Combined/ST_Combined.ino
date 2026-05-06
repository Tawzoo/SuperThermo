#include <Wire.h>
#include <SPI.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_MAX31856.h>

// Gy 906 sensor (infrared sensor)
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

float ambientOffset = 0.0;
float objectOffset  = 0.0;

float smoothedObject = 0;
float alpha = 0.15;   // Lower = smoother, higher = faster response

// MAX31856 K-TYPE THERMOCOUPLE
// Max31856 K-type Thermocouple
#define MAX31856_CS 10

Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(MAX31856_CS);

// Timing
unsigned long startTime;
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 60000;  // 1 min

float readObjectAverage(int samples = 10) {
  float sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += mlx.readObjectTempC();
    delay(20);
  }

  return (sum / samples) + objectOffset;
}

float readAmbientAverage(int samples = 10) {
  float sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += mlx.readAmbientTempC();
    delay(20);
  }

  return (sum / samples) + ambientOffset;
}

void setup() {
  Serial.begin(9600);

  // Start I2C for GY-906
  Wire.begin();

  // Start Gy 906 sensor
  if (!mlx.begin()) {
    Serial.println("ERROR: GY-906 / MLX90614 not found. Check I2C wiring.");
    while (1);
  }

  // Start MAX31856
  if (!maxthermo.begin()) {
    Serial.println("ERROR: MAX31856 thermocouple module not found. Check SPI wiring.");
    while (1);
  }

  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);

  smoothedObject = readObjectAverage();

  startTime = millis();

  Serial.println("Combined Temperature Monitoring System Ready");
  Serial.println("GY-906 IR Sensor + MAX31856 K-Type Thermocouple");
  Serial.println("------------------------------------------------");
  Serial.println("Time(s), IR_Ambient(C), IR_Object(C), IR_Smoothed(C), Thermocouple(C), Cold_Junction(C)");
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastPrintTime >= printInterval) {
    lastPrintTime = currentTime;

    float irAmbient = readAmbientAverage();
    float irObject = readObjectAverage();

    smoothedObject = alpha * irObject + (1 - alpha) * smoothedObject;

    float thermocoupleTemp = maxthermo.readThermocoupleTemperature();
    float coldJunctionTemp = maxthermo.readCJTemperature();

    unsigned long runTimeSeconds = (currentTime - startTime) / 1000;

    // Check MAX31856 fault status
    uint8_t fault = maxthermo.readFault();

    Serial.print(runTimeSeconds);
    Serial.print(", ");

    Serial.print(irAmbient, 2);
    Serial.print(", ");

    Serial.print(irObject, 2);
    Serial.print(", ");

    Serial.print(smoothedObject, 2);
    Serial.print(", ");

    Serial.print(thermocoupleTemp, 2);
    Serial.print(", ");

    Serial.print(coldJunctionTemp, 2);

    if (fault) {
      Serial.print(", FAULT: ");

      if (fault & MAX31856_FAULT_CJRANGE) Serial.print("Cold Junction Range ");
      if (fault & MAX31856_FAULT_TCRANGE) Serial.print("Thermocouple Range ");
      if (fault & MAX31856_FAULT_CJHIGH)  Serial.print("Cold Junction High ");
      if (fault & MAX31856_FAULT_CJLOW)   Serial.print("Cold Junction Low ");
      if (fault & MAX31856_FAULT_TCHIGH)  Serial.print("Thermocouple High ");
      if (fault & MAX31856_FAULT_TCLOW)   Serial.print("Thermocouple Low ");
      if (fault & MAX31856_FAULT_OVUV)    Serial.print("Over/Under Voltage ");
      if (fault & MAX31856_FAULT_OPEN)    Serial.print("Thermocouple Open ");
    }

    Serial.println();
  }
}