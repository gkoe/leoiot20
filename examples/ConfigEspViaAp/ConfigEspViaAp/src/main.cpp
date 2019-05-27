#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspAp.h>
#include <EspConfig.h>
#include <HttpServer.h>

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
  EspAp.init();
  while (!EspAp.isApStarted())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  char ntpServer[LENGTH_SHORT_TEXT];
  EspConfig.getNvsStringValue("ntpserver", ntpServer);
  sprintf(loggerMessage, "SSID:%s, ThingName:%s, NtpServer:%s, MqttBroker:%s:%i", EspConfig.getSsid(), EspConfig.getThingName(), 
              ntpServer, EspConfig.getMqttBroker(), EspConfig.getMqttBrokerPort());
  Logger.info("ConfigEspViaAp", loggerMessage);
  Logger.info("ConfigEspViaAp", "Connect with AP from ESP_xxx");
  HttpServer.init();
  Logger.info("!!! Config WiFi", "http://192.168.10.1/setconfig?ssid=SSID&password=PASSWORD");
  Logger.info("!!! Config thing and MQTT", "http://192.168.10.1/setconfig?mqttbroker=192.168.0.1&mqttport=1883&thingname=demo");
  Logger.info("!!! Check config:", "http://192.168.10.1/getconfig");
}
