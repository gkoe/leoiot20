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
#include <EspMqttClient.h>
#include <SystemService.h>
#include <EspUdp.h>
#include <UdpLoggerTarget.h>
#include <Thing.h>

extern "C"
{
  void app_main(void);
}

#include <SimplePir.h>
const gpio_num_t PIR_PIN = GPIO_NUM_26;

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("==========\n");
  printf("CarControl\n");
  printf("==========\n");
  EspConfig.init();
  const char *thingName = EspConfig.getThingName();
  Logger.init("CarControl");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("CarControl, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("CarControl, app_main()", "HttpServer started");
  EspTime.init();
  EspMqttClient.init(thingName);
  EspUdp.init();
  Thing.init();
  Logger.info("CarControl, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
