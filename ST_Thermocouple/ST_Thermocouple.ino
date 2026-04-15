#include <Adafruit_MAX31856.h>

#define CS_PIN 10

Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(CS_PIN);

void setup() {
  Serial.begin(9600);
  maxthermo.begin();
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
}

void loop() {
  Serial.print("Temp C = ");
  Serial.println(maxthermo.readThermocoupleTemperature());
  delay(1000);
}