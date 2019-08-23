#include "Bmp280_Humidity.h"
#include <Logger.h>

Bmp280_Humidity::Bmp280_Humidity(Bmp280Sensor* bmp, const char* thingName, const char* name,const char* unit, float threshold, float minValue, float maxValue) 
		:IotSensor( thingName, name, unit, threshold, minValue, maxValue)
{
	_bmp = bmp;
}

void Bmp280_Humidity::measure()
{
	float humidity = _bmp->getHumidity();
	if(humidity < 0.0 || humidity > 100.0){
		// char loggerMessage[LENGTH_LOGGER_MESSAGE];
		// sprintf(loggerMessage, "measure(), illegal value: %f", humidity);
		// Logger.debug("Dht22_Humidity", loggerMessage);
		return;
	}
	setMeasurement(humidity);
}
