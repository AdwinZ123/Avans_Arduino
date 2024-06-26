#include "Temperatuursensor.h"
#include <Arduino.h>

Temperatuursensor::Temperatuursensor(int pin) {
  _pin = pin;
}

Temperatuursensor::~Temperatuursensor() {}

float Temperatuursensor::GetTemperatuur() {
  // Lees de analoge waarde van de sensor
  int sensorValue = analogRead(_pin);

  // Converteer de analoge waarde naar millivolts (0-1023 naar 0-5000mV)
  float voltage = sensorValue * (5000.0 / 1023.0);

  // Converteer de spanning naar temperatuur in Celsius
  float  temperature = voltage / 10.0;

  // Print de temperatuur naar de seriële monitor
  Serial.print("Temperatuur: ");
  Serial.print(temperature);
  Serial.println(" °C");

  return temperature;
}

float Temperatuursensor::MeetGemiddeldeTemperatuur(){
  float totaleTemperatuur = 0.00;

  for (int i = 0; i < 5; i++) {
    float temp = GetTemperatuur();
    totaleTemperatuur = (totaleTemperatuur + temp);
    delay(500);
  }

  float gemiddelde = (totaleTemperatuur / 5);

  Serial.print("Gemiddelde: ");
  Serial.print(gemiddelde);
  Serial.print(" Celsius \n");
  

  return gemiddelde;
};