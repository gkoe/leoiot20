#include "Dht22_Humidity.h"
#include <Logger.h>

Dht22_Humidity::Dht22_Humidity(Dht22* dht, const char* nodeName, const char* name,const char* unit, float threshold, float minValue, float maxValue) 
		:IotSensor( nodeName, name, unit, threshold, minValue, maxValue)
{
	_dht = dht;
}

void Dht22_Humidity::measure()
{
	float humidity = _dht->getHumidity();
	setMeasurement(humidity);
}
