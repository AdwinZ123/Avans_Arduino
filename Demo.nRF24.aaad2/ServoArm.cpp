#include "ServoArm.h"
#include <Arduino.h>
#include <Servo.h>

ServoArm::ServoArm(int interval) {
  _updateInterval = interval;
  _increment = 1;

  _stapjes = 5;
  _pos = 0;
  _nextPos = 0;
  _klapUitPositie = 95;
  _klapInPositie = 0;
  _running = false;

  _minPos = 0;
  _maxPos = 180;
}

ServoArm::~ServoArm() {}

void ServoArm::Attach(int pin) {
  _servo.attach(pin);

  _nextPos = 0;
  _pos = 0;
  _running = true;

  _servo.write(0);
  delay(1000);
}

void ServoArm::Detach() {
  _servo.detach();
}

void ServoArm::Update() {
  if ((millis() - _lastUpdate) > _updateInterval && _pos != _nextPos && _running)  // time to update
  {
    Serial.print("Servo update \n");
    _lastUpdate = millis();

    if ((_pos >= _nextPos)) {
      _increment = -1;
    } else {
      _increment = 1;
    }

    _pos += _increment;
    _servo.write(_pos);  // funtie aanroepen voor positie
    Serial.println(_pos);

    if (_pos == _nextPos) {
      _running = false;
    }
  }
}

void ServoArm::KlapUit() {
  _nextPos = _klapUitPositie;
  _running = true;
}

void ServoArm::KlapIn() {
  _nextPos = _klapInPositie;
  _pos = _klapInPositie;
  _servo.write(_pos);
  delay(500);
}

void ServoArm::Omhoog() {
  Serial.print("omhoog \n");
  if (_nextPos >= _minPos) {
    _nextPos -= _stapjes;
    _running = true;
  }
}

void ServoArm::Omlaag() {
  Serial.print("omlaag \n");
  if (_nextPos <= _maxPos) {
    _nextPos += _stapjes;
    _running = true;
  }
}
