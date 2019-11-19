#include "Dht22_Humidity.h"
#include <Logger.h>

Dht22_Humidity::Dht22_Humidity(Dht22* dht, const char* thingName, const char* name,const char* unit, float threshold, float minValue, float maxValue, bool getAverageValue) 
		:IotSensor( thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
	_dht = dht;
}

void Dht22_Humidity::measure()
{
	float humidity = _dht->getHumidity();
	if (humidity != -1000)
	{
		setMeasurement(humidity);
	}
	
	
}
