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

#include <HX711.h>
#include <driver/adc.h>
#include <Power.h>
#include "driver/gpio.h"
#include <PowerSwitch.h>



// HX711 circuit wiring
const gpio_num_t HX711_DOUT_PIN = GPIO_NUM_23;
const gpio_num_t HX711_SCK_PIN = GPIO_NUM_22;
// 
const gpio_num_t SSR_POWER_PUMP = GPIO_NUM_15;
const gpio_num_t SSR_POWER_VALVE = GPIO_NUM_13;

const adc1_channel_t POWER_ADC_CHANNEL = ADC1_CHANNEL_6;  // GPIO34


extern "C"
{
  void app_main(void);
}

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("===========\n");
  printf("Cistern\n");
  printf("===========\n");
  EspConfig.init();
  const char *thingName = EspConfig.getThingName();
  Logger.init("Cistern");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("Cistern, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("Cistern, app_main()", "HttpServer started");
  EspTime.init();
  EspUdp.init();
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  Logger.info("Cistern, app_main()", "Init Mqtt-Connection");
  EspMqttClient.init(thingName);
  while (!EspMqttClient.isMqttConnected())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  Thing.init();
  Logger.info("Cistern, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  IotSensor *hx711 = new HX711(HX711_DOUT_PIN, HX711_SCK_PIN, thingName, "weight", "kg", 1, true);
  Thing.addSensor(hx711);
  Power *powerPtr = new Power(POWER_ADC_CHANNEL, "power", "power", "W", 2);
  Thing.addSensor(powerPtr);
  IotActor *powerPump = new PowerSwitch(SSR_POWER_PUMP, false, thingName, "powerpump");
  Thing.addActor(powerPump);
  IotActor *powerValve = new PowerSwitch(SSR_POWER_VALVE, false, thingName, "powervalve");
  Thing.addActor(powerValve);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil
  EspMqttClient.addSubscriptionsToBroker();

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
