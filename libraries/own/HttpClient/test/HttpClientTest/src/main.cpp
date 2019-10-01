#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspStation.h>
#include <HttpServer.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <SystemService.h>

#include <HttpClient.h>

extern "C"
{
  void app_main(void);
}

const char *SERIAL_LOGGER_TAG = "SLT";

const char *urlMifloras = "leonding.synology.me/esplogs/mifloraentries";

void app_main()
{
  printf("!!! ===============\n");
  printf("!!! HttpClient Test\n");
  printf("!!! ===============\n");
  EspConfig.init();
  Logger.init("HttpClientTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("HttpClientTest, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("HttpClientTest, app_main()", "HttpServer started");
  EspTime.init();

  char payload[LENGTH_PAYLOAD];

  HttpClient.get(urlMifloras, payload, LENGTH_PAYLOAD-1, true,"gerald", "piKla87Sie57");
  printf("!!! GET with https and basic authentication to %s, payload: %s\n", urlMifloras, payload);

  sprintf(payload, "{\"mac\": \"mac12345\",\"moisture\": 55.5,\"temperature\": 39.9,\"brightness\": 999,\"batteryLevel\": 99.9}");

  HttpClient.post(urlMifloras, payload,true,"gerald", "piKla87Sie57");

  while (true)
  {
    SystemService.checkSystem();
    vTaskDelay(1);
  }
}
