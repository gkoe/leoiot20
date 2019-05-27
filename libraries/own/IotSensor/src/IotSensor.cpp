#include "IotSensor.h"
#include <EspTime.h>
#include <EspMqttClient.h>
#include <EspConfig.h>
#include <string.h>
#include <Logger.h>

IotSensor::IotSensor(const char *thingName, const char *name, const char *unit, float threshold, int maxIntervall)
{
	strcpy(_thingName, thingName);
	strcpy(_name, name);
	strcpy(_unit, unit);
	_threshold = threshold;
	_maxIntervall=maxIntervall;
	_publishedMeasurement = 0;
	_lastMeasurement = 0;
	_time = EspTime.getTime();
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	sprintf(loggerMessage, "Sensor initialized: %s", name);
	Logger.info("Sensor;Constructor", loggerMessage);
}

/**
 * Der konkrete Sensor meldet einen Messwert. 
 * Es wird überprüft, ob der Messwert gemeldet werden muss (Zeitablauf, Änderung)
 * und bei Bedarf per MQTT gemeldet.
 */
void IotSensor::setMeasurement(float value)
{
	_lastMeasurement = value;
	float delta = value - _publishedMeasurement;
	if(delta < 0.0){
		delta = delta * (-1.0);
	} 
	long time = EspTime.getTime();
	if (time > _time && (delta >= _threshold || time > _time + _maxIntervall))  // nicht in gleicher Sekunde mehrere Werte publishen
	{
		char loggerMessage[LENGTH_LOGGER_MESSAGE];
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
		Logger.info("Sensor;set Measurement", loggerMessage);
		EspMqttClient.publish(fullTopic, payload);
		sprintf(loggerMessage, "%s: %.1f%s,Time: %ld",_name, _publishedMeasurement, _unit, _time);
		Logger.info("Sensor;set Measurement", loggerMessage);
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

void IotSensor::getMqttPayload(char *payload, float measurement)
{
	sprintf(payload, "{\"timestamp\":%ld,\"value\":%.2f}", EspTime.getTime(), measurement);
}

//void IotSensor::measure(){}
