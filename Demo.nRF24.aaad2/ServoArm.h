#include <Servo.h>

#ifndef SERVOARM_H
#define SERVOARM_H

class ServoArm{
  private:
    Servo _servo;

    int _pos;              // current servo position 
    int _increment;        // increment to move for each interval
    int  _updateInterval;      // interval between updates
    unsigned long _lastUpdate; // last update of position
    
  public:
		ServoArm(int);
		~ServoArm();

    // void KlapUit();
    // void KlapIn();
    // void NaarLinks();
    // void NaarRechts();

  void Attach(int);
  void Detach();
  void Update();
};

#endif