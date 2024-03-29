#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "IotActor.h"
#include <EspTime.h>
#include <IotSensor.h>
#include <Thing.h>
#include <Logger.h>
#include <EspConfig.h>

#define MESSAGE_LENGTH 100
//#define Actor_DEBUG

/**
 * Das eingehende Topic wird geparst. Es enthält zuerst die Thingadresse,
 * dann den Actorname gefolgt vom konstanten Text command.
 * 		/wohnzimmer/light/command
 * Die Payload enthält dann die Werte (Standardmäßig ON/OFF)
 */
static void actorMqttCallback(const char *topic, const char *payload)
{
	// Actor aus topic extrahieren
	int thingNameLength = strlen(EspConfig.getThingName());
	int topicLength = strlen(topic);
	int actorNameLength = topicLength - thingNameLength - 9; // /command am Ende (8 Zeichen) wird nicht mitkopiert
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	char actorName[LENGTH_SENSOR_ACTOR_NAME];

	sprintf(loggerMessage, "Topiclength: %il , ThingNameLength: %i , ActorNameLength: %i",
			topicLength, thingNameLength, actorNameLength);
	Logger.info("Actor Mqtt Callback", loggerMessage);
	strncpy(actorName, topic + thingNameLength + 1, actorNameLength);
	actorName[actorNameLength] = 0;
	sprintf(loggerMessage, "Actorname: '%s'", actorName);
	Logger.info("Actor Mqtt Callback", loggerMessage);
	IotActor *actorPtr = Thing.getActorByName(actorName);
	actorPtr->setStateByMqtt(payload);
}

IotActor::IotActor(const char *thingName, const char *name)
{
	strcpy(_thingName, thingName);
	strcpy(_name, name);
	_time = EspTime.getTime();
	strcpy(_settedState, "0");
	strcpy(_currentState, "0");
	strcpy(_lastReportedState, "0");
	strcpy(_actorTopic, EspConfig.getThingName());
	strcat(_actorTopic, "/");
	strcat(_actorTopic, getName());
	_actorSubscription.topic = _actorTopic;
	_actorSubscription.subscriberCallback = actorMqttCallback;
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	sprintf(loggerMessage, "Subscribe for Topic: %s", _actorTopic);
	Logger.info("Actor Construct:", loggerMessage);
	EspMqttClient.addSubscription(&_actorSubscription);
}

/**
 * Aus der empfangenen Mqtt-Message ist der zu setzende
 * Wert des Aktors zu übernehmen und im Aktor zu setzen.
 * Beim nächsten Synchronisationsvorgang wird der
 * Aktor dann entsprechend angesteuert.
 */
void IotActor::setStateByMqtt(const char *payload)
{
	// {"value":999.9}
	setState(payload); // später auf JSON anpassen
}

void IotActor::setState(const char *stateText)
{
	portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&myMutex);
	//critical section
	strncpy(_settedState, stateText, LENGTH_STATE);
	portEXIT_CRITICAL(&myMutex);
	// sync();
}

char *IotActor::getSettedState()
{
	return _settedState;
}
char *IotActor::getName()
{
	return _name;
}

/*
		Der aktuelle Zustand des Aktors wird an den gesetzten Wert 
		angepasst. Bei binären Zuständen wird der Aktor aufgefordert,
		auf den neuen Zustand umzuschalten. Bei länger andauernden
		Schaltvorgängen (z.B. Rollladen) wird der Zielwert mit
		der Zeit erreicht.
		Der Aktor meldet bei Veränderung des Istwertes die Änderung.
*/
void IotActor::sync()
{
	if (strcmp(_currentState, _settedState) != 0)
	{
		char loggerMessage[LENGTH_LOGGER_MESSAGE];
		// snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "SyncState, currentState: %s, settedState: %s",
		// 		 _currentState, _settedState);
		// Logger.info("Actor Sync", loggerMessage);
		setActor(_settedState);
		if (strcmp(_currentState, _lastReportedState) != 0)
		{
			char fullTopic[LENGTH_TOPIC];
			sprintf(fullTopic, "%s/state", _name);
			char payload[LENGTH_PAYLOAD];
			sprintf(payload, "{\"timestamp\":%ld,\"value\":%s}", EspTime.getTime(), _currentState);
			sprintf(loggerMessage, "Topic: %s, Payload: %s", fullTopic, payload);
			Logger.info("Actor Sync", loggerMessage);
			EspMqttClient.publish(fullTopic, payload);
			strcpy(_lastReportedState, _currentState);
		}
	}
}

char *IotActor::getCurrentState()
{
	return _currentState;
}

void IotActor::setCurrentState(const char *state)
{
	portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&myMutex);
	//critical section
	strcpy(_currentState, state);
	portEXIT_CRITICAL(&myMutex);
}
