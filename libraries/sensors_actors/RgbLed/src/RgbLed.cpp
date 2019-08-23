#include <RgbLed.h>
#include <Thing.h>
#include <Logger.h>

RgbLed::RgbLed(const gpio_num_t pinR, const gpio_num_t pinG, const gpio_num_t pinB, const char *thingName,
			   const char *name) : IotActor(thingName, name)
{
	_pinR = pinR;
	_pinG = pinG;
	_pinB = pinB;
	gpio_pad_select_gpio(_pinR);
	gpio_set_direction(_pinR, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(_pinG);
	gpio_set_direction(_pinG, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(_pinB);
	gpio_set_direction(_pinB, GPIO_MODE_OUTPUT);
	setCurrentState("0");
	Logger.debug("RgbLed", "Constructor");
}

void RgbLed::setActor(const char *newState)
{
	char message[LENGTH_LOGGER_MESSAGE];
	snprintf(message, LENGTH_LOGGER_MESSAGE, "setActor: %s", newState);
	Logger.debug("RgbLed", message);
	int value = atoi(newState);
	if (value >= 100)
	{
		gpio_set_level(_pinR, 1);
	}
	else
	{
		gpio_set_level(_pinR, 0);
	}
	value = value %100;
	if (value >= 10)
	{
		gpio_set_level(_pinG, 1);
	}
	else
	{
		gpio_set_level(_pinG, 0);
	}
	value = value %10;
	if (value >= 1)
	{
		gpio_set_level(_pinB, 1);
	}
	else
	{
		gpio_set_level(_pinB, 0);
	}
	setCurrentState(newState);
}
