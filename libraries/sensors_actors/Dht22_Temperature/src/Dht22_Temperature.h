#pragma once

#include <Dht22.h>
#include <IotSensor.h>

class Dht22_Temperature : public IotSensor
{
public:
	Dht22_Temperature(Dht22* dht, const char* thingName, const char* name, const char* unit, float threshold,  float minValue = -9999.9, float maxValue = 9999.9, bool getAverageValue = true);
	virtual void measure();
private :
	Dht22* _dht;
};
