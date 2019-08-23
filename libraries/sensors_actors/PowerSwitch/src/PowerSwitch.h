#pragma once
#include "esp_system.h"

#include <IotActor.h>

/*
	PowerSwitch ist ein spezieller sehr einfacher Aktor als
	Ein/Ausschalter über einen digitalen Ausgang.
*/
class PowerSwitch : public IotActor
{
 public:
	 PowerSwitch(const gpio_num_t pinNumber, const bool isInverse, const char* thingName, const char* name);
	/**
	 * Der konkrete Aktor ändert den Zustand entsprechend
	 * z.B. schaltet den Schalter ein/aus
	 */
	virtual void setActor(const char* newState);
	
private:
	gpio_num_t _pinNumber;
	bool _isInverse;
};

