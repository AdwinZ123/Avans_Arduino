#include "Stappenmotor.h"
#include <Arduino.h>

Stappenmotor::Stappenmotor(int interval) {
  _interval = interval;
  _maxSteps = 200;
  _steps = 1;
}

Stappenmotor::~Stappenmotor() {}

void Stappenmotor::Attach(int stepPin, int dirPin, int enPin) {
  _stepPin = stepPin;
  _dirPin = dirPin;
  _enPin = enPin;
  _enabled = true;

  // Activate actuators
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  digitalWrite(dirPin, HIGH);
}

void Stappenmotor::Update() {
  // Serial.print("1 \n");
  // currentMillis = millis();
  // if (currentMillis - previousStepperMillis >= stepperTime) {
  //   // Stepper motor
  //   digitalWrite(DIRPIN, HIGH);  // Enables the motor to move in a particular direction
  //   for (int x = 0; x < 800; x++) {
  //     Serial.print("Loop 1 \n");
  //     digitalWrite(STEPPIN, HIGH);
  //     delayMicroseconds(200);
  //     digitalWrite(STEPPIN, LOW);
  //     delayMicroseconds(200);
  //   }

  //   digitalWrite(DIRPIN, LOW);  //Changes the direction of rotation
  //   for (int x = 0; x < 800; x++) {
  //     Serial.print("Loop 2 \n");
  //     digitalWrite(STEPPIN, HIGH);
  //     delayMicroseconds(200);
  //     digitalWrite(STEPPIN, LOW);
  //     delayMicroseconds(200);
  //   }
  // }


  if (_steps <= _maxSteps && (millis() - _lastUpdate) >= _interval)  // time to update
  {

    Serial.print("2 \n");
    _lastUpdate = millis();

    if (digitalRead(_stepPin) == HIGH) {
      Serial.print("3 \n");
      if (_steps <= _maxSteps) {
        Serial.print("4 \n");
        _steps += _steps;
      } else {
        Serial.print("5 \n");
        digitalWrite(_stepPin, HIGH);
      }
    } else {
      Serial.print("6 \n");
      digitalWrite(_stepPin, LOW);
    }



    // pos += increment;
    // servo.write(pos);
    // Serial.println(pos);
    // if ((pos >= 180) || (pos <= 0))  // end of sweep
    // {
    //   // reverse direction
    //   increment = -increment;
    // }
  }
}