#include <PowerSwitch.h>
#include <Thing.h>
#include <Logger.h>

/*
	Der Schalter hat keine Einheit des Messwertes EIN/AUS und auch eine fixe Schwelle von 0.001,
	da sowieso nur die Werte 0 und 1 verwendet werden.
*/
PowerSwitch::PowerSwitch(const gpio_num_t pinNumber, const bool isInverse, const char *thingName, const char *name) : IotActor(thingName, name)
{
	_pinNumber = pinNumber;
	_isInverse = isInverse;

	gpio_pad_select_gpio(_pinNumber);
	gpio_set_direction(_pinNumber, GPIO_MODE_OUTPUT);

	if (isInverse)
	{
		gpio_set_level(_pinNumber, 1);
	}
	else
	{
		gpio_set_level(_pinNumber, 0);
	}
	setCurrentState("0");
}

void PowerSwitch::setActor(const char *newState)
{
	bool isOn = (strcmp(newState, "ON") == 0) || (strcmp(newState, "on") == 0) || (strcmp(newState, "1") == 0);
	if (_isInverse)
	{
		if (isOn)
		{
			Logger.info("PowerSwitch;setActor()", "1, Pin low!");
			gpio_set_level(_pinNumber, 0);
		}
		else
		{
			Logger.info("PowerSwitch;setActor()", "0, Pin high!");
			gpio_set_level(_pinNumber, 1);
		}
	}
	else
	{ // normale (nicht inverse Logik)
		if (isOn)
		{
			Logger.info("PowerSwitch;setActor()", "1, Pin high!");
			gpio_set_level(_pinNumber, 1);
		}
		else
		{
			Logger.info("PowerSwitch;setActor()", "0, Pin low!");
			gpio_set_level(_pinNumber, 0);
		}
	}
	setCurrentState(newState);
}
