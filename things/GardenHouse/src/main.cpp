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
#include <Contact.h>
#include <Distance.h>

extern "C"
{
  void app_main(void);
}

const gpio_num_t ULTRASONIC_TRIGGER_PIN = GPIO_NUM_18;
const gpio_num_t ULTRASONIC_ECHO_PIN = GPIO_NUM_19;
const gpio_num_t CONTACT_PIN = GPIO_NUM_13;
const gpio_num_t POWER_LEFT = GPIO_NUM_12;
const gpio_num_t POWER_MIDDLE = GPIO_NUM_14;
const gpio_num_t POWER_RIGHT = GPIO_NUM_27;

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("===========\n");
  printf("Gardenhouse\n");
  printf("===========\n");
  EspConfig.init();
  const char *thingName = EspConfig.getThingName();
  Logger.init("GardenHouse");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("Gardenhouse, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("Gardenhouse, app_main()", "HttpServer started");
  EspTime.init();
  EspUdp.init();
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  Logger.info("Gardenhouse, app_main()", "Init Mqtt-Connection");
  EspMqttClient.init(thingName);
  while (!EspMqttClient.isMqttConnected())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  Thing.init();
  Logger.info("Gardenhouse, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  // IotSensor *waterEmptyContact =
  //     new Contact(CONTACT_PIN, thingName, "watercontact", "", 0.2);
  // Thing.addSensor(waterEmptyContact);
  IotSensor *waterlevel =
      new Distance(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, thingName, "waterlevel", "cm", 0.5, 10.0, 100.0, true);
  Thing.addSensor(waterlevel);
  IotActor *powerLeft =
      new PowerSwitch(POWER_LEFT, false, thingName, "powerpump");
  Thing.addActor(powerLeft);
  IotActor *powerMiddle =
      new PowerSwitch(POWER_MIDDLE, false, thingName, "powermiddle");
  Thing.addActor(powerMiddle);
  IotActor *powerRight =
      new PowerSwitch(POWER_RIGHT, false, thingName, "powerright");
  Thing.addActor(powerRight);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil
  EspMqttClient.addSubscriptionsToBroker();

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
