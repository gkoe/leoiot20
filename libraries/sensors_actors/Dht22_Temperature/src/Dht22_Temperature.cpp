#include "Dht22_Temperature.h"
#include <Logger.h>

Dht22_Temperature::Dht22_Temperature(Dht22* dht, const char* thingName, 
			const char* name, const char* unit, float threshold) 
		:IotSensor(thingName, name, unit, threshold)
{
	_dht = dht;
}

void Dht22_Temperature::measure()
{
	float temperature = _dht->getTemperature();
	if(temperature < -20.0 || temperature > 80.0){
		// char loggerMessage[LENGTH_LOGGER_MESSAGE];
		// sprintf(loggerMessage, "measure(), illegal value: %f", temperature);
		// Logger.debug("Dht22_Temperature", loggerMessage);
		return;
	}

	setMeasurement(temperature);
}
