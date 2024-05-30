#include "Temperatuursensor.h"
#include <Arduino.h>

Temperatuursensor::Temperatuursensor(){}

Temperatuursensor::~Temperatuursensor() {}

void Temperatuursensor::begin(int pin) {
  _pin = pin;
	pinMode(pin,OUTPUT);
}

bool Temperatuursensor::getState() {
	// return state of led
	return (_ledState);
}

void Temperatuursensor::setState(bool state) {
	// switch led on or off
	_ledState = state;
	digitalWrite(_pin,state);
}