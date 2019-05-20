#include <PowerSwitchSensor.h>

PowerSwitchSensor::PowerSwitchSensor(const char *nodeName, const char *name, const char *unit, float threashold, uint8_t pinNumber)
	: Sensor(nodeName, name, unit, threashold)
{
	char loggerMessage[75];
	sprintf(loggerMessage, "Sensor initialized: %s", name);
	Logger.info("PowerSwitchSensor Constructor", loggerMessage);
	_pinNumber = pinNumber;
}

void PowerSwitchSensor::measure()
{
	// inverse Logik
	if (digitalRead(_pinNumber) == 0)
	{
		// Serial.println(F("*PS: PowerSwitchSensor low ==> return 1"));
		setMeasurement(1);
	}
	else
	{
		setMeasurement(0);
		// Serial.println(F("*PS: PowerSwitchSensor high ==> return 0"));
	}
}
