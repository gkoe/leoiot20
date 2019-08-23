#pragma once

#include <Dht22.h>
#include <IotSensor.h>

class Dht22_Humidity : public IotSensor
{
public:
	Dht22_Humidity(Dht22* dht, const char* nodeName, const char* name, const char* unit, float threshold, float minValue, float maxValue);
	virtual void measure();
private:
	Dht22* _dht;
};
