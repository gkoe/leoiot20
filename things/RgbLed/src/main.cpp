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
#include <RgbLed.h>


extern "C"
{
  void app_main(void);
}


const gpio_num_t R = GPIO_NUM_13;
const gpio_num_t G = GPIO_NUM_12;
const gpio_num_t B = GPIO_NUM_14;

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("=================\n");
  printf("Thing with RgbLed\n");
  printf("=================\n");
  EspConfig.init();
  const char *thingName = EspConfig.getThingName();
  Logger.init("ThingTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("ThingTest, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("ThingTest, app_main()", "HttpServer started");
  EspTime.init();
  EspMqttClient.init(thingName);
  EspUdp.init();
  Thing.init();
  Logger.info("ThingTest, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
	IotActor* rgbLedPtr =
		 new RgbLed(R,G,B, thingName, "rgbled");
	Thing.addActor(rgbLedPtr);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
