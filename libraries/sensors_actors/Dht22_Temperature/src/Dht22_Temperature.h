#pragma once

#include <Dht22.h>
#include <IotSensor.h>

class Dht22_Temperature : public IotSensor
{
public:
	Dht22_Temperature(Dht22* dht, const char* nodeName, const char* name, const char* unit, float threshold);
	virtual void measure();
private:
	Dht22* _dht;
};
