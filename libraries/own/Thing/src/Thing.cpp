#include "Thing.h"
#include <EspMqttClient.h>
#include <EspConfig.h>
#include <HttpServer.h>
#include <Logger.h>

// bool splitPair(char *text, const char *delimiter, char *first, char *second)
// {
// 	char *equalPtr;
// 	equalPtr = strtok(text, delimiter);
// 	first = equalPtr;
// 	equalPtr = strtok(NULL, delimiter);
// 	second = equalPtr;
// 	return (first == nullptr) || (second == nullptr);
// }

/**
 * eigene http-Route zum Setzen eines Aktorwertes  /setactor?rgbled=010
 */
static esp_err_t setActorRequestHandler(httpd_req_t *req)
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	char queryString[LENGTH_MIDDLE_TEXT];
	char response[LENGTH_MIDDLE_TEXT];
	esp_err_t err;
	int queryLength = httpd_req_get_url_query_len(req) + 1;
	if (queryLength > 0)
	{
		err = httpd_req_get_url_query_str(req, queryString, queryLength);
		if (err == ESP_OK)
		{
			if (queryString != NULL)
			{
				char *actorName;
				char *value = nullptr;
				char *rest = queryString;
				actorName = strtok_r(rest, "=", &rest);
				bool ok = true;
				if (actorName)
				{
					value = strtok_r(rest, "=", &rest);
					if (!value)
					{
						ok = false;
					}
				}
				else
				{
					ok = false;
				}
				if (ok)
				{
					sprintf(loggerMessage, "Splitted: %s %s", actorName, value);
					Logger.info("Thing,setActorRequestHandler()", loggerMessage);
					IotActor *actor = Thing.getActorByName(actorName);
					if (actor == nullptr)
					{
						sprintf(response, "Actor %s not found!", actorName);
						sprintf(loggerMessage, "Response: %s", response);
						Logger.info("Thing,setActorRequestHandler()", loggerMessage);
						httpd_resp_send(req, response, strlen(response));
					}
					else
					{
						actor->setState(value);
						sprintf(response, "Actors's %s set from %s to %s", actorName, actor->getCurrentState(), actor->getSettedState());
						sprintf(loggerMessage, "Response: %s", response);
						Logger.info("Thing,setActorRequestHandler()", loggerMessage);
						httpd_resp_send(req, response, strlen(response));
					}
				}
				else
				{
					sprintf(response, "No actor=value in querystring: %s", queryString);
					sprintf(loggerMessage, "Response: %s", response);
					Logger.info("Thing,setActorRequestHandler()", loggerMessage);
					httpd_resp_send(req, response, strlen(response));
				}
			}
			else
			{
				sprintf(response, "QUERYSTRING IS NULL");
				sprintf(loggerMessage, "Response: %s", response);
				Logger.error("Thing, setActorRequestHandler()", loggerMessage);
				httpd_resp_send(req, response, strlen(response));
			}
		}
		else
		{
			sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
			Logger.error("Thing, setActorRequestHandler()", loggerMessage);
			httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
		}
	}
	else
	{
		sprintf(loggerMessage, "NO QUERYSTRING");
		Logger.info("Thing, setActorRequestHandler(), QueryString:", "NO QUERYSTRING");
		httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
	}
	return ESP_OK;
}

/**
 * eigene http-Route zum Abfragen eines Sensorwertes  /getsensor?temperature
 */
static esp_err_t getSensorRequestHandler(httpd_req_t *req)
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	char sensorName[LENGTH_MIDDLE_TEXT];
	esp_err_t err;
	int queryLength = httpd_req_get_url_query_len(req) + 1;
	if (queryLength > 0)
	{
		err = httpd_req_get_url_query_str(req, sensorName, queryLength);
		if (err == ESP_OK)
		{
			if (sensorName != NULL)
			{
				IotSensor *sensor = Thing.getSensorByName(sensorName);
				char response[LENGTH_MIDDLE_TEXT];
				if (sensor == NULL)
				{
					sprintf(response, "Sensor %s not found!", sensorName);
					httpd_resp_send(req, response, strlen(response));
				}
				else
				{
					float measurement = sensor->getLastMeasurement();
					sprintf(response, "Sensor's %s value %.2f %s", sensorName, measurement, sensor->getUnit());
					sprintf(loggerMessage, "Response: %s", response);
					Logger.info("Thing,getSensorRequestHandler()", loggerMessage);
					httpd_resp_send(req, response, strlen(response));
				}
			}
		}
		else
		{
			sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
			Logger.error("Thing, getSensorRequestHandler()", loggerMessage);
			httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
		}
	}
	else
	{
		sprintf(loggerMessage, "NO QUERYSTRING");
		Logger.info("Thing, getSensorRequestHandler(), QueryString:", "NO QUERYSTRING");
		httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
	}
	return ESP_OK;
}

/**
 * eigene http-Route zum Abfragen eines Aktorwertes  /getactor?switch
 */
static esp_err_t getActorStateRequestHandler(httpd_req_t *req)
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	char actorName[LENGTH_MIDDLE_TEXT];
	esp_err_t err;
	int queryLength = httpd_req_get_url_query_len(req) + 1;
	if (queryLength > 0)
	{
		err = httpd_req_get_url_query_str(req, actorName, queryLength);
		if (err == ESP_OK)
		{
			if (actorName != NULL)
			{
				IotActor *sensor = Thing.getActorByName(actorName);
				char response[LENGTH_MIDDLE_TEXT];
				if (sensor == NULL)
				{
					sprintf(response, "Actor %s not found!", actorName);
					httpd_resp_send(req, response, strlen(response));
				}
				else
				{
					char *state = sensor->getCurrentState();
					sprintf(response, "Actors's %s state %s", actorName, state);
					sprintf(loggerMessage, "Response: %s", response);
					Logger.info("Thing,getActorStateRequestHandler()", loggerMessage);
					httpd_resp_send(req, response, strlen(response));
				}
			}
		}
		else
		{
			sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
			Logger.error("Thing, getActorStateRequestHandler()", loggerMessage);
			httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
		}
	}
	else
	{
		sprintf(loggerMessage, "NO QUERYSTRING");
		Logger.info("Thing, getActorStateRequestHandler(), QueryString:", "NO QUERYSTRING");
		httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
	}
	return ESP_OK;
}

static const httpd_uri_t getsensorrequest = {
	.uri = "/getsensor",
	.method = HTTP_GET,
	.handler = getSensorRequestHandler,
	.user_ctx = nullptr};

static const httpd_uri_t getactorrequest = {
	.uri = "/getactor",
	.method = HTTP_GET,
	.handler = getActorStateRequestHandler,
	.user_ctx = nullptr};

static const httpd_uri_t setactorrequest = {
	.uri = "/setactor",
	.method = HTTP_GET,
	.handler = setActorRequestHandler,
	.user_ctx = nullptr};

/*
 * Thing wird initialisiert und Routen zum Lesen von Sensorwerten, bzw. Setzen
 * von Aktorwerten werden registriert.
 */
void ThingClass::init()
{
	Logger.info("Thing, init(), name: ", EspConfig.getThingName());
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	sprintf(loggerMessage, "Thing init with name: %s", EspConfig.getThingName());
	Logger.info("ThingClass Init", loggerMessage);
	HttpServer.addRoute(&getsensorrequest);
	HttpServer.addRoute(&setactorrequest);
	HttpServer.addRoute(&getactorrequest);
}

void ThingClass::addSensor(IotSensor *sensorPtr)
{
	_sensors.push_back(sensorPtr);
}

void ThingClass::addActor(IotActor *actorPtr)
{
	_actors.push_back(actorPtr);
}

void ThingClass::refreshSensorsAndActors()
{
	for (std::list<IotSensor *>::iterator it = _sensors.begin(); it != _sensors.end(); ++it)
	{
		// Serial.print(F("*NO: refresh, Sensor: "));
		// Serial.print(it->first);
		// Serial.print(F(", last Value: "));
		// Serial.println(it->second->getLastMeasurement());
		IotSensor *sensorPtr = *it;
		sensorPtr->measure();
	}
	for (std::list<IotActor *>::iterator it = _actors.begin(); it != _actors.end(); ++it)
	{
		// Serial.print(F("*NO: refresh, Actor: "));
		// Serial.print(it->first);
		// Serial.print(F(", last Value: "));
		// Serial.println(it->second->getLastMeasurement());
		(*it)->sync();
	}
}

IotActor *ThingClass::getActorByName(char *name)
{
	for (std::list<IotActor *>::iterator it = _actors.begin(); it != _actors.end(); ++it)
	{
		// Serial.print(F("*NO: refresh, Sensor: "));
		// Serial.print(it->first);
		// Serial.print(F(", last Value: "));
		// Serial.println(it->second->getLastMeasurement());
		IotActor *actorPtr = *it;
		if (strcmp(actorPtr->getName(), name) == 0)
		{
			return actorPtr;
		}
	}
	return nullptr;
}

IotSensor *ThingClass::getSensorByName(char *name)
{
	for (std::list<IotSensor *>::iterator it = _sensors.begin(); it != _sensors.end(); ++it)
	{
		IotSensor *sensorPtr = *it;
		if (strcmp(sensorPtr->getName(), name) == 0)
		{
			return sensorPtr;
		}
	}
	return nullptr;
}

void ThingClass::getAllSensorName(char *names)
{
	bool x = false;
	for (std::list<IotSensor *>::iterator it = _sensors.begin(); it != _sensors.end(); ++it)
	{
		x = true;
		IotSensor *sensorPtr = *it;
		strcat(names, sensorPtr->getName());
		strcat(names, ";");
	}
	if (x)
	{
		Logger.info("Thing Get Sensor Name", names);
	}
	else
	{
		Logger.info("Thing Get Sensor Name", "No Sensors");
	}
}

void ThingClass::getAllActorName(char *names)
{
	bool x = false;
	for (std::list<IotActor *>::iterator it = _actors.begin(); it != _actors.end(); ++it)
	{
		x = true;
		IotActor *actorPtr = *it;
		strcat(names, actorPtr->getName());
		strcat(names, ";");
	}
	if (x)
	{
		Logger.info("Thing Get Actor Name", names);
	}
	else
	{
		Logger.info("Thing Get Actor Name", "No Actor");
	}
}

ThingClass Thing;
