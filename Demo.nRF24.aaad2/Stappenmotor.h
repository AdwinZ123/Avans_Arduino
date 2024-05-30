#ifndef STAPPENMOTOR_H
#define STAPPENMOTOR_H

class Stappenmotor{
  private:
// #define STEPPIN 5
// #define DIRPIN 2
// #define ENPIN 8
    int _stepPin;
    int _dirPin;
    int _enPin;

    bool _direction;
    bool _enabled;
    int _steps;
    int _maxSteps;
    int _interval;
    unsigned long _lastUpdate;
    
  public:
		Stappenmotor(int);
		~Stappenmotor();

    void Attach(int, int, int);
		void Update();
};

#endif