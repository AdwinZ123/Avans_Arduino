#include <Servo.h>

#ifndef SERVOARM_H
#define SERVOARM_H

class ServoArm {
private:
  Servo _servo;

  int _pos;                   // current servo position
  int _increment;             // increment to move for each interval
  int _updateInterval;        // interval between updates
  unsigned long _lastUpdate;  // last update of position

  int _klapUitPositie;
  int _klapInPositie;
  int _stapjes;
  int _nextPos;
  bool _running;
  int _minPos;
  int _maxPos;

public:
  ServoArm(int);
  ~ServoArm();

  void KlapUit();
  void KlapIn();
  void Omhoog();
  void Omlaag();

  void Attach(int);
  void Detach();
  void Update();
};

#endif