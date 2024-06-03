#ifndef WATERLEVELSENSOR_H
#define WATERLEVELSENSOR_H

class WaterLevelSensor {
private:

public:
  WaterLevelSensor();
  ~WaterLevelSensor();

  void Attach();

  void GetHigh12SectionValue(void);
  void GetLow8SectionValue(void);
  int GetWaterLevelPercentage();
};

#endif