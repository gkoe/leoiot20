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
	if (value == -1000)
	{
		return;
	}

	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	char averageText[LENGTH_SHORT_TEXT];
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
	_lastMeasurementTime = EspTime.getTime();
	// Mittelwertsbildung
	// if (_lastValues[_actLastValuesIndex] != value) // hintereinander gemessene gleiche Werte aus Mittelwertsbildung unterdrücken
	// 			// weil setMeasurement() in sehr kurzen Zeitabständen den aktuellen Messwert überträgt
	// {
	_actLastValuesIndex++;
	if (_actLastValuesIndex >= 10)
	{
		_actLastValuesIndex = 0;
	}
	_lastValues[_actLastValuesIndex] = value;
	// }
	if (_getAverageValue)
	{
		value = getAverageValue();
	}

	float delta = value - _publishedMeasurement;
	if (delta < 0.0)
	{
		delta = delta * (-1.0);
	}
	// sprintf(loggerMessage, "Array: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
	// 		_lastValues[0], _lastValues[1], _lastValues[2], _lastValues[3], _lastValues[4], _lastValues[5], _lastValues[6], _lastValues[7], _lastValues[8], _lastValues[9]);
	// Logger.verbose("Sensor;set Measurement", loggerMessage);
	// sprintf(loggerMessage, "Vor Pruefung %s value: %.2f, delta: %.2f, threshold: %.2f, Time: %ld, Last: %ld",
	// 		_name, value, delta, _threshold, time, _time);
	// Logger.verbose("Sensor;set Measurement", loggerMessage);
	time = EspTime.getTime();
	if (value > _minValue && value < _maxValue &&
		time > _time && (delta >= _threshold || time > _time + _maxIntervall)) // nicht in gleicher Sekunde mehrere Werte publishen
	{
		if (_getAverageValue)
		{
			strcpy(averageText, "Durchschnitt");
		}
		else
		{
			strcpy(averageText, "Einzelwert");
		}

		sprintf(loggerMessage, "Neuer Messwert (%s) fuer %s: %.2f, avg von: %.2f%s auf %.2f%s, Time: %ld, Last: %ld",
				averageText, _name, _lastMeasurement, _publishedMeasurement, _unit, value, _unit, time, _time);
		Logger.info("Sensor;set Measurement", loggerMessage);
		_publishedMeasurement = value;
		_time = time;
		char fullTopic[LENGTH_TOPIC];
		//!sprintf(fullTopic, "%s/%s", Thing.getName(), _name);
		//Serial.println(_thingName);
		sprintf(fullTopic, "%s/state", _name);
		char payload[LENGTH_TOPIC];
		getMqttPayload(payload, value);
		// sprintf(loggerMessage, "Topic: %s, Payload: %s", fullTopic, payload);
		// Logger.verbose("Sensor;set Measurement", loggerMessage);
		EspMqttClient.publish(fullTopic, payload);
		sprintf(loggerMessage, "%s: %.2f %s,Time: %ld", _name, _publishedMeasurement, _unit, _time);
		Logger.verbose("Sensor;set Measurement", loggerMessage);
	}
}

float IotSensor::getLastMeasurement()
{
	return _lastMeasurement;
}

long IotSensor::getLastMeasurementTime()
{
	return _lastMeasurementTime;
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

/**
 * Von den letzten 10 Messwerten werden die größten zwei und die kleinsten zwei Werte gestrichen.
 * Aus den restlichen (maximal 6) Werten wird der Mittelwert gebildet.
 */
float IotSensor::getAverageValue()
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	float actLowestValue = _maxValue;
	float actSecondLowestValue = _maxValue;
	float actGreatestValue = _minValue;
	float actSecondGreatestValue = _minValue;
	int validValues = 0;
	float sumOfValues = 0;
	for (int i = 0; i < 10; i++)
	{
		if (_lastValues[i] != _minValue)
		{
			float value = _lastValues[i];
			if (value > actSecondGreatestValue)
			{
				if (value > actGreatestValue)
				{
					actSecondGreatestValue = actGreatestValue;
					actGreatestValue = value;
				}
				else
				{
					actSecondGreatestValue = value;
				}
			}
			if (value < actSecondLowestValue)
			{
				if (value < actLowestValue)
				{
					actSecondLowestValue = actLowestValue;
					actLowestValue = value;
				}
				else
				{
					actSecondLowestValue = value;
				}
			}
			sumOfValues += value;
			validValues++;
		}
	}
	if (validValues < 5)
		return -1;
	// sprintf(loggerMessage, "sumOfValues: %.2f, maxValue: %.2f, minValue: %.2f, actLowestValue: %.2f, actSecondLowestValue: %.2f actGreatestValue: %.2f, actSecondGreatestValue: %.2f validValues: %d",
	// 	   sumOfValues, _maxValue, _minValue, actLowestValue, actSecondLowestValue, actGreatestValue, actSecondGreatestValue, validValues);
	// Logger.verbose("Sensor;getAverageValue()", loggerMessage);
	return (sumOfValues - actLowestValue - actSecondLowestValue - actGreatestValue - actSecondGreatestValue) / (validValues - 4);
}
