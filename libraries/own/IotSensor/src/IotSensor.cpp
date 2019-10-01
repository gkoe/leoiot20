#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "IotSensor.h"
#include <EspTime.h>
#include <EspMqttClient.h>
#include <EspConfig.h>
#include <string.h>
#include <Logger.h>

IotSensor::IotSensor(const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
{
	_getAverageValue = getAverageValue;
	strcpy(_thingName, thingName);
	strcpy(_name, name);
	strcpy(_unit, unit);
	_threshold = threshold;
	_maxIntervall = MAX_INTERVALL;
	_minValue = minValue;
	_maxValue = maxValue;
	_publishedMeasurement = 0;
	_lastMeasurement = 0;
	_time = EspTime.getTime();
	for (int i = 0; i < 10; i++)
	{
		_lastValues[i] = minValue;
	}
	_actLastValuesIndex = 0;
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	sprintf(loggerMessage, "Sensor initialized: %s", name);
	Logger.info("Sensor;Constructor", loggerMessage);
}

void IotSensor::setMaxIntervall(int intervall)
{
	_maxIntervall = intervall;
}

/**
 * Der konkrete Sensor meldet einen Messwert. 
 * Es wird überprüft, ob der Messwert gemeldet werden muss (Zeitablauf, Änderung)
 * und bei Bedarf per MQTT gemeldet.
 */
void IotSensor::setMeasurement(float value)
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	long time = EspTime.getTime();
	if ((value < _minValue) || (value > _maxValue))
	{
		if (time - _lastIllegalValueTime < MIN_ILLEGAL_VALUE_TIMESPAN)
		{
			return;
		}
		_lastIllegalValueTime = time;
		if (value < _minValue)
		{
			sprintf(loggerMessage, "%s, Illegal value: %.1f lower than minValue %.1f", _name, value, _minValue);
			Logger.error("Sensor;set Measurement", loggerMessage);
			return;
		}
		if (value > _maxValue)
		{
			sprintf(loggerMessage, "%s, Illegal value: %.1f greater than maxValue %.1f", _name, value, _maxValue);
			Logger.error("Sensor;set Measurement", loggerMessage);
			return;
		}
	}

	_lastMeasurement = value;
	// Mittelwertsbildung
	if (_lastValues[_actLastValuesIndex] != value)  // hinter einander gemessene gleiche Werte aus Mittelwertsbildung unterdrücken
	{
		_actLastValuesIndex++;
		if (_actLastValuesIndex >= 10)
		{
			_actLastValuesIndex = 0;
		}
		_lastValues[_actLastValuesIndex] = value;
	}
	if (_getAverageValue)
	{
		value = getAverageValue();
	}

	float delta = value - _publishedMeasurement;
	if (delta < 0.0)
	{
		delta = delta * (-1.0);
	}
	time = EspTime.getTime();
	if (value > _minValue && value < _maxValue &&
		  time > _time && (delta >= _threshold || time > _time + _maxIntervall)) // nicht in gleicher Sekunde mehrere Werte publishen
	{
		sprintf(loggerMessage, "Neuer Messwert fuer %s: %.1f%s auf %.1f%s, Time: %ld, Last: %ld", _name, _publishedMeasurement, _unit, value, _unit, time, _time);
		Logger.info("Sensor;set Measurement", loggerMessage);
		_publishedMeasurement = value;
		_time = time;
		char fullTopic[LENGTH_TOPIC];
		//!sprintf(fullTopic, "%s/%s", Thing.getName(), _name);
		//Serial.println(_thingName);
		sprintf(fullTopic, "%s/state", _name);
		char payload[LENGTH_TOPIC];
		getMqttPayload(payload, value);
		sprintf(loggerMessage, "Topic: %s, Payload: %s", fullTopic, payload);
		Logger.verbose("Sensor;set Measurement", loggerMessage);
		EspMqttClient.publish(fullTopic, payload);
		sprintf(loggerMessage, "%s: %.1f %s,Time: %ld", _name, _publishedMeasurement, _unit, _time);
		Logger.verbose("Sensor;set Measurement", loggerMessage);
	}
}

float IotSensor::getLastMeasurement()
{
	return _lastMeasurement;
}

void IotSensor::measure()
{
}

char *IotSensor::getName()
{
	return _name;
}

char *IotSensor::getUnit()
{
	return _unit;
}

void IotSensor::getMqttPayload(char *payload, float measurement)
{
	sprintf(payload, "{\"timestamp\":%ld,\"value\":%.2f}", EspTime.getTime(), measurement);
}

//void IotSensor::measure(){}

bool IotSensor::getPinState(gpio_num_t pin)
{
	return gpio_input_get() & (1 << pin);
}

float IotSensor::getAverageValue()
{
	int actMinValue = _maxValue;
	int actMaxValue = _minValue;
	int validValues = 0;
	int sumOfValues = 0;
	for (int i = 0; i < 10; i++)
	{
		if (_lastValues[i] != _minValue)
		{
			uint32_t value = _lastValues[i];
			if (value > actMaxValue)
			{
				actMaxValue = value;
			}
			else if (value < actMinValue)
			{
				actMinValue = value;
			}
			sumOfValues += value;
			validValues++;
		}
	}
	if (validValues < 3)
		return -1;
	//printf("sumOfValues: %d, minValue: %d, maxValue: %d, validValues: %d\n", sumOfValues, actMinValue, actMaxValue, validValues);
	return (sumOfValues - actMinValue - actMaxValue) / ((float)validValues - 2);
}
