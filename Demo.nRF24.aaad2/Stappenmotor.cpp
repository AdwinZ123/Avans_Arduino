#include "Stappenmotor.h"
#include <Arduino.h>
#include <AccelStepper.h>

Stappenmotor::Stappenmotor(int motorInterfaceType, int stepPin, int dirPin, int enPin) {
  _stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
  _stepper.setMaxSpeed(1000);
  _stepper.setAcceleration(100);


  _motorInterfaceType = motorInterfaceType;
  _stepPin = stepPin;
  _dirPin = dirPin;
  _enPin = enPin;

  _stapjes = 20;
  _huidigePositie = 0;
  _klapUitPositie = -200;
  _klapInPositie = 0;

  _minPos = 0;
  _maxPos = -240;
}

Stappenmotor::~Stappenmotor() {}

void Stappenmotor::SetZeroPosition(int buttonPin) {
  while (digitalRead(buttonPin) == LOW) {
    int huidigePositie = _stepper.currentPosition();
    _stepper.moveTo(huidigePositie + 15);
    _stepper.runToPosition();
  }

  _huidigePositie = 0;
  _stepper.setCurrentPosition(0);
  Serial.print("Stepper current position: ");
  Serial.print(_stepper.currentPosition());
  Serial.print("\n");
}

void Stappenmotor::KlapUit() {
  _huidigePositie = _klapUitPositie;
  _stepper.moveTo(_klapUitPositie);
  _stepper.runToPosition();

  Serial.print("Stepper current position: ");
  Serial.print(_stepper.currentPosition());
  Serial.print("\n");
}

void Stappenmotor::KlapIn() {
  _huidigePositie = _klapInPositie;
  _stepper.moveTo(_klapInPositie);
  _stepper.runToPosition();

  Serial.print("Stepper current position: ");
  Serial.print(_stepper.currentPosition());
  Serial.print("\n");
}

void Stappenmotor::NaarLinks() {
  Serial.print("links \n");
  if (_huidigePositie > _maxPos) {
    _huidigePositie -= _stapjes;
    _stepper.moveTo(_huidigePositie);
    _stepper.runToPosition();

    Serial.print("Stepper current position: ");
    Serial.print(_stepper.currentPosition());
    Serial.print("\n");
  }
}

void Stappenmotor::NaarRechts() {
  Serial.print("rechts \n");
  if (_huidigePositie < _minPos) {
    _huidigePositie += _stapjes;
    _stepper.moveTo(_huidigePositie);
    _stepper.runToPosition();

    Serial.print("Stepper current position: ");
    Serial.print(_stepper.currentPosition());
    Serial.print("\n");
  }
}
