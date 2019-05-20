#include "Dht22_Humidity.h"
#include <Logger.h>

Dht22_Humidity::Dht22_Humidity(Dht22* dht, const char* nodeName, const char* name,const char* unit, float threshold) 
		:IotSensor( nodeName, name, unit, threshold)
{
	_dht = dht;
}

void Dht22_Humidity::measure()
{
	float humidity = _dht->getHumidity();
	if(humidity < 0.0 || humidity > 100.0){
		// char loggerMessage[LENGTH_LOGGER_MESSAGE];
		// sprintf(loggerMessage, "measure(), illegal value: %f", humidity);
		// Logger.debug("Dht22_Humidity", loggerMessage);
		return;
	}
	setMeasurement(humidity);
}
