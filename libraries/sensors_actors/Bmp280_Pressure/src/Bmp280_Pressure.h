#pragma once

#include <Bmp280Sensor.h>
#include <IotSensor.h>

class Bmp280_Pressure : public IotSensor
{
public:
	Bmp280_Pressure(Bmp280Sensor* bmp, const char* nodeName, const char* name, const char* unit, float threshold, float minValue, float maxValue);
	virtual void measure();
private:
	Bmp280Sensor* _bmp;
};
