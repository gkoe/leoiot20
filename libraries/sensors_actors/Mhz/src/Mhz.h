#pragma once
#include "driver/gpio.h"
#include <IotSensor.h>

class Mhz : public IotSensor
{
public:
  Mhz(gpio_num_t rxPin, gpio_num_t txPin, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue=true);
  float readCo2FromMhz();
  float getCo2();
  void calibrate();
  char getCheckSum(char *packet);
  float _ppmCo2 = -1.0;

private:
  uint8_t *_data;
  virtual void measure();
  uint8_t _cmd_measure_co2[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  uint8_t _calibrate[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
  uint8_t mhzCmdABCEnable[9] = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};
  uint8_t mhzCmdABCDisable[9] = {0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86};
  unsigned long _lastMeasurementMilliSeconds = 0;
};
