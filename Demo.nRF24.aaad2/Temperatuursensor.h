#ifndef TEMPERATUURSENSOR_H
#define TEMPERATUURSENSOR_H

class Temperatuursensor{
  private:
		bool _ledState;
    int _pin;
    
  public:
		Temperatuursensor();
		~Temperatuursensor();
		
		void begin(int);		

		bool getState();
		void setState(bool);
};

#endif