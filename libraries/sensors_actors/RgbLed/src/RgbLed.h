#pragma once

#include <IotActor.h>
#include "driver/gpio.h"


/*
	RgbLed ist ein Aktor, dem über einen Floatvalue die Helligkeit für Rot, Grün und
	Blau so übermittelt werden, dass die Einer/Zehnerstelle Blau in %, die Hunderter/Tausenderstelle Grün
	und die Zehntausender/Hunderttausenderstelle Rot in Prozent angibt.
	Die Helligkeit wird über PWM gesteuert
*/
class RgbLed : public IotActor
{
public:
	RgbLed(const gpio_num_t pinR, const gpio_num_t pinG, const gpio_num_t pinB, 
			const char *thingName, const char *name);

	virtual void setActor(const char* newState);

	// const char *Red = "990000";
	// const char *Green = "9900";
	// const char *Blue = "99";
	// const char *Off = "00";

private:
	gpio_num_t _pinR;
	gpio_num_t _pinG;
	gpio_num_t _pinB;
	// int _ledChannelR = 1;	// IO-Pin wird per PWM angesteuert
	// int _ledChannelG = 2;
	// int _ledChannelB = 3;
};
