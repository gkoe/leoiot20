#include "Bmp280_Temperature.h"
#include <Logger.h>

Bmp280_Temperature::Bmp280_Temperature(Bmp280Sensor* bmp, const char* thingName, const char* name,const char* unit, float threshold, float minValue, float maxValue) 
		:IotSensor( thingName, name, unit, threshold, minValue, maxValue)
{
	_bmp = bmp;
}

void Bmp280_Temperature::measure()
{
	float temperature = _bmp->getTemperature();
	if(temperature < -40.0 || temperature > 150.0){
		return;
	}
	setMeasurement(temperature);
}
