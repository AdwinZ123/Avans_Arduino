#include <AccelStepper.h>

#ifndef STAPPENMOTOR_H
#define STAPPENMOTOR_H

class Stappenmotor {
private:
  AccelStepper _stepper;

  int _stepPin;
  int _dirPin;
  int _enPin;
  int _motorInterfaceType;

  int _klapUitPositie;
  int _klapInPositie;
  int _stapjes;
  int _huidigePositie;
  int _minPos;
  int _maxPos;

public:
  Stappenmotor(int, int, int, int);
  ~Stappenmotor();

  void SetZeroPosition(int);

  void KlapUit();
  void KlapIn();
  void NaarLinks();
  void NaarRechts();
};

#endif