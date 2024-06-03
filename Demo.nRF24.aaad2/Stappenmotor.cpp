#include "Stappenmotor.h"
#include <Arduino.h>
#include <AccelStepper.h>

Stappenmotor::Stappenmotor(int motorInterfaceType, int stepPin, int dirPin, int enPin) {
  _stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
  _stepper.setCurrentPosition(0);
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
}

Stappenmotor::~Stappenmotor() {}

void Stappenmotor::KlapUit() {
  _huidigePositie += _klapUitPositie;
  _stepper.moveTo(_klapUitPositie);
  _stepper.runToPosition();
}

void Stappenmotor::KlapIn() {
  _huidigePositie = _klapInPositie;
  _stepper.moveTo(_klapInPositie);
  _stepper.runToPosition();
}

void Stappenmotor::NaarLinks() {
  _huidigePositie -= _stapjes;
  _stepper.moveTo(_huidigePositie);
  _stepper.runToPosition();
}

void Stappenmotor::NaarRechts() {
  _huidigePositie += _stapjes;
  _stepper.moveTo(_huidigePositie);
  _stepper.runToPosition();
}
