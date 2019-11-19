#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspAp.h>
#include <EspStation.h>
#include <EspConfig.h>
#include <HttpServer.h>
#include <SystemService.h>

extern "C"
{
  void app_main(void);
}

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("=================\n");
  printf("Config ESP via AP\n");
  printf("=================\n");
  EspConfig.init();
  Logger.init("ConfigEspViaAp");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_INFO);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  // EspStation.init();
  // Logger.info("ConfigEspViaAp, app_main()", "Waiting for connection as station!");
  // int waitingMilliseconds = 5000;
  // while (waitingMilliseconds > 0 && !EspStation.isStationOnline())
  // {
  //   vTaskDelay(1 / portTICK_PERIOD_MS);
  //   waitingMilliseconds--;
  // }
  // if (!EspStation.isStationOnline())
  // {
    EspAp.init();
    while (!EspAp.isApStarted())
    {
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
  // }

  char ntpServer[LENGTH_SHORT_TEXT];
  EspConfig.getNvsStringValue("ntpserver", ntpServer);
  sprintf(loggerMessage, "SSID:%s, Password:%s ThingName:%s, NtpServer:%s, MqttBroker:%s:%i", EspConfig.getSsid(), EspConfig.getPassword(), EspConfig.getThingName(),
          ntpServer, EspConfig.getMqttBroker(), EspConfig.getMqttBrokerPort());
  Logger.info("ConfigEspViaAp", loggerMessage);
  Logger.info("ConfigEspViaAp", "Connect with AP from ESP_xxx");
  HttpServer.init();

  while (true)
  {
    SystemService.checkSystem();
    vTaskDelay(1);
  }
}
