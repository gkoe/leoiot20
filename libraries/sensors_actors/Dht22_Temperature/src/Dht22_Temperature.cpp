#include "Dht22_Temperature.h"
#include <Logger.h>

Dht22_Temperature::Dht22_Temperature(Dht22 *dht, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
	: IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
	_dht = dht;
}

void Dht22_Temperature::measure()
{
	float temperature = _dht->getTemperature();
	if (temperature != -1000)
	{
		setMeasurement(temperature);
	}
}
