#include "Bmp280_Pressure.h"
#include <Logger.h>

Bmp280_Pressure::Bmp280_Pressure(Bmp280Sensor* bmp, const char* thingName, const char* name,const char* unit, float threshold, float minValue, float maxValue) 
		:IotSensor( thingName, name, unit, threshold, minValue, maxValue)
{
	_bmp = bmp;
}

void Bmp280_Pressure::measure()
{
	float pressure = _bmp->getPressure();
	setMeasurement(pressure);
}
