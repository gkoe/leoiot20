#pragma once

#include <Dht22.h>
#include <IotSensor.h>

class Dht22_Humidity : public IotSensor
{
public:
	Dht22_Humidity(Dht22* dht, const char* thingName, const char* name, const char* unit, float threshold, float minValue = 0.0, float maxValue = 100.0, bool getAverageValue = true);
	virtual void measure();
private:
	Dht22* _dht;
};
