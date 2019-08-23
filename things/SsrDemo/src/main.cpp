#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "driver/gpio.h"

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspStation.h>
#include <HttpServer.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <EspMqttClient.h>
#include <SystemService.h>
#include <EspUdp.h>
#include <UdpLoggerTarget.h>
#include <Thing.h>

#include <PowerSwitch.h>

extern "C"
{
  void app_main(void);
}

const gpio_num_t SSR_SWITCH = GPIO_NUM_12;

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("===========\n");
  printf("SsrDemo\n");
  printf("========\n");
  EspConfig.init();
  const char *thingName = EspConfig.getThingName();
  Logger.init("SsrDemo");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("SsrDemo, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("SsrDemo, app_main()", "HttpServer started");
  EspTime.init();
  EspUdp.init();
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  Logger.info("SsrDemo, app_main()", "Init Mqtt-Connection");
  EspMqttClient.init(thingName);
  while (!EspMqttClient.isMqttConnected())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  Thing.init();
  Logger.info("SsrDemo, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  IotActor *ssrSwitch =
      new PowerSwitch(SSR_SWITCH, false, thingName, "switch");
  Thing.addActor(ssrSwitch);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil
  EspMqttClient.addSubscriptionsToBroker();

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
