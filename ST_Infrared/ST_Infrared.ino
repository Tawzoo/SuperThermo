#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Calibration offsets
float ambientOffset = 0.0;
float objectOffset  = 0.0;

// Smoothing
float smoothedObject = 0;
float alpha = 0.15;   // lower = smoother, higher = faster response

float readObjectAverage(int samples = 20) {
  float sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += mlx.readObjectTempC();
    delay(30);
  }

  return (sum / samples) + objectOffset;
}

float readAmbientAverage(int samples = 20) {
  float sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += mlx.readAmbientTempC();
    delay(30);
  }

  return (sum / samples) + ambientOffset;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!mlx.begin()) {
    Serial.println("GY-906 / MLX90614 not found. Check wiring.");
    while (1);
  }

  Serial.println("GY-906 Temperature Gradient System Ready");
  Serial.println("----------------------------------------");

  smoothedObject = readObjectAverage();
}

void loop() {
  float ambientTemp = readAmbientAverage();
  float objectTemp = readObjectAverage();

  // Exponential smoothing
  smoothedObject = alpha * objectTemp + (1 - alpha) * smoothedObject;

  Serial.print("Ambient: ");
  Serial.print(ambientTemp, 2);
  Serial.print(" °C | Object: ");
  Serial.print(objectTemp, 2);
  Serial.print(" °C | Smoothed Object: ");
  Serial.print(smoothedObject, 2);
  Serial.println(" °C");

  delay(1000);
}