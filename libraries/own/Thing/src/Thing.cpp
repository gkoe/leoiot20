#include "Thing.h"
#include <EspMqttClient.h>
#include <EspConfig.h>
#include <HttpServer.h>
#include <EspTime.h>
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
 * eigene http-Route zum Lesen /actor?rgbled und Setzen eines Aktorwertes  /actor?rgbled=010
 * Abfrage aller Actornamen über /actor
 */
static esp_err_t actorRequestHandler(httpd_req_t *req)
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	char queryString[LENGTH_MIDDLE_TEXT];
	char response[LENGTH_MIDDLE_TEXT];
	esp_err_t err;
	int queryLength = httpd_req_get_url_query_len(req) + 1;
	if (queryLength > 1)  // gibt es überhaupt Parameter ==> speziellen Actor lesen oder setzen
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
				if (actorName)
				{
					sprintf(loggerMessage, "Actorname: %s", actorName);
					Logger.info("Thing,actorRequestHandler()", loggerMessage);
					IotActor *actor = Thing.getActorByName(actorName);
					if (actor == nullptr)
					{
						sprintf(response, "Actor %s not found!", actorName);
						sprintf(loggerMessage, "Response: %s", response);
						Logger.info("Thing,actorRequestHandler()", loggerMessage);
						httpd_resp_send(req, response, strlen(response));
					}
					else // Actor existiert
					{
						value = strtok_r(rest, "=", &rest);
						if (!value) // Abfrage des Actors
						{
							char *state = actor->getCurrentState();
							sprintf(response, "Actors's %s state %s", actorName, state);
							sprintf(loggerMessage, "Response: %s", response);
							Logger.info("Thing,actorStateRequestHandler()", loggerMessage);
							httpd_resp_send(req, response, strlen(response));
						}
						else // Setzen des Actors
						{
							actor->setState(value);
							sprintf(response, "Actors's %s set from %s to %s", actorName, actor->getCurrentState(), actor->getSettedState());
							sprintf(loggerMessage, "Response: %s", response);
							Logger.info("Thing,actorRequestHandler()", loggerMessage);
							httpd_resp_send(req, response, strlen(response));
						}
					}
				}  // kein Actorname im Querystring
				else
				{
					sprintf(response, "No actor=value in querystring: %s", queryString);
					sprintf(loggerMessage, "Response: %s", response);
					Logger.info("Thing,actorRequestHandler()", loggerMessage);
					httpd_resp_send(req, response, strlen(response));
				}
			}
			else
			{
				sprintf(response, "QUERYSTRING IS NULL");
				sprintf(loggerMessage, "Response: %s", response);
				Logger.error("Thing, actorRequestHandler()", loggerMessage);
				httpd_resp_send(req, response, strlen(response));
			}
		}
		else
		{
			sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
			Logger.error("Thing, actorRequestHandler()", loggerMessage);
			httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
		}
	}
	else
	{
		char loggerMessage[LENGTH_LOGGER_MESSAGE];
		Thing.getAllActorNames(loggerMessage);
		if (strlen(loggerMessage) == 0)
		{
			sprintf(loggerMessage, "No Actor in thing");
		}
		
		httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
		Logger.info("HttpServer, actorStateRequestHandler(), Actors:", loggerMessage);
	}
	return ESP_OK;
}

/**
 * eigene http-Route zum Abfragen eines Sensorwertes  /getsensor?temperature
 */
static esp_err_t sensorRequestHandler(httpd_req_t *req)
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	char sensorName[LENGTH_MIDDLE_TEXT];
	esp_err_t err;
	int queryLength = httpd_req_get_url_query_len(req) + 1;
	if (queryLength > 1)
	{
		err = httpd_req_get_url_query_str(req, sensorName, queryLength);
		if (err == ESP_OK)
		{
			if (sensorName != NULL)
			{
				IotSensor *sensor = Thing.getSensorByName(sensorName);
				char response[LENGTH_PAYLOAD];
				if (sensor == NULL)
				{
					sprintf(response, "Sensor %s not found!", sensorName);
					httpd_resp_send(req, response, strlen(response));
				}
				else
				{
					float measurement = sensor->getLastMeasurement();
					long time = sensor->getLastMeasurementTime();
					char timeString[LENGTH_SHORT_TEXT];
					EspTime.getDateTimeString(time, timeString);
					sprintf(response, "{\"sensor\": %s,\"time\": %s,\"value\": %.2f %s}", sensorName, timeString, measurement, sensor->getUnit());
					sprintf(loggerMessage, "Response: %s", response);
					Logger.info("This()", loggerMessage);
					httpd_resp_send(req, response, strlen(response));
				}
			}
		}
		else
		{
			sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
			Logger.error("Thins()", loggerMessage);
			httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
		}
	}
	else
	{
		char loggerMessage[LENGTH_LOGGER_MESSAGE];
		Thing.getAllSensorNames(loggerMessage);
		if (strlen(loggerMessage) == 0)
		{
			sprintf(loggerMessage, "No Sensor in thing");
		}
		
		httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
		Logger.info("HttpServer, getSensorHandler(), Sensors:", loggerMessage);
	}
	return ESP_OK;
}

// /**
//  * eigene http-Route zum Abfragen eines Aktorwertes  /getactor?switch
//  */
// static esp_err_t actorStateRequestHandler(httpd_req_t *req)
// {
// 	char loggerMessage[LENGTH_LOGGER_MESSAGE];
// 	char actorName[LENGTH_MIDDLE_TEXT];
// 	esp_err_t err;
// 	int queryLength = httpd_req_get_url_query_len(req) + 1;
// 	if (queryLength > 1)
// 	{
// 		err = httpd_req_get_url_query_str(req, actorName, queryLength);
// 		if (err == ESP_OK)
// 		{
// 			if (actorName != NULL)
// 			{
// 				IotActor *actor = Thing.getActorByName(actorName);
// 				char response[LENGTH_MIDDLE_TEXT];
// 				if (actor == NULL)
// 				{
// 					sprintf(response, "Actor %s not found!", actorName);
// 					httpd_resp_send(req, response, strlen(response));
// 				}
// 				else
// 				{
// 					char *state = actor->getCurrentState();
// 					sprintf(response, "Actors's %s state %s", actorName, state);
// 					sprintf(loggerMessage, "Response: %s", response);
// 					Logger.info("Thing,getActorStateRequestHandler()", loggerMessage);
// 					httpd_resp_send(req, response, strlen(response));
// 				}
// 			}
// 		}
// 		else
// 		{
// 			sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
// 			Logger.error("Thing, getActorStateRequestHandler()", loggerMessage);
// 			httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
// 		}
// 	}
// 	else
// 	{
// 		char loggerMessage[LENGTH_LOGGER_MESSAGE];
// 		Thing.getAllActorNames(loggerMessage);
// 		httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
// 		Logger.info("HttpServer, getActorStateRequestHandler(), Actors:", loggerMessage);

// 		// sprintf(loggerMessage, "NO QUERYSTRING");
// 		// Logger.info("Thing, getActorStateRequestHandler(), QueryString:", "NO QUERYSTRING");
// 		// httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
// 	}
// 	return ESP_OK;
// }

static const httpd_uri_t sensorrequest = {
	.uri = "/sensor",
	.method = HTTP_GET,
	.handler = sensorRequestHandler,
	.user_ctx = nullptr};

// static const httpd_uri_t getactorrequest = {
// 	.uri = "/getactor",
// 	.method = HTTP_GET,
// 	.handler = getActorStateRequestHandler,
// 	.user_ctx = nullptr};

static const httpd_uri_t actorrequest = {
	.uri = "/actor",
	.method = HTTP_GET,
	.handler = actorRequestHandler,
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
	HttpServer.addRoute(&sensorrequest);
	HttpServer.addRoute(&actorrequest);
	// HttpServer.addRoute(&getactorrequest);
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

void ThingClass::getAllSensorNames(char *names)
{
	bool x = false;
	names[0] = 0;
	for (std::list<IotSensor *>::iterator it = _sensors.begin(); it != _sensors.end(); ++it)
	{
		x = true;
		IotSensor *sensorPtr = *it;
		strcat(names, sensorPtr->getName());
		strcat(names, ";");
	}
	if (x)
	{
		Logger.info("Thing; getAllSensorNames()", names);
	}
	else
	{
		Logger.info("Thing; getAllSensorNames()", "No Sensors");
	}
}

void ThingClass::getAllActorNames(char *names)
{
	bool x = false;
	names[0] = 0;
	for (std::list<IotActor *>::iterator it = _actors.begin(); it != _actors.end(); ++it)
	{
		x = true;
		IotActor *actorPtr = *it;
		strcat(names, actorPtr->getName());
		strcat(names, ";");
	}
	if (x)
	{
		Logger.info("Thing getAllActorNames()", names);
	}
	else
	{
		Logger.info("Thing getAllActorNames()", "No Actor");
	}
}

ThingClass Thing;
