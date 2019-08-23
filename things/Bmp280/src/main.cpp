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
#include <Bmp280Sensor.h>
// #include <Bmp280_Temperature.h> // Wrapper über den Dht22 zur Einbindung als Temperatursensor
#include <Bmp280_Humidity.h>
#include <Bmp280_Temperature.h>
#include <Bmp280_Pressure.h>

extern "C"
{
  void app_main(void);
}

const gpio_num_t SDA_GPIO = GPIO_NUM_21;
const gpio_num_t SCL_GPIO = GPIO_NUM_22;

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("=================\n");
  printf("Thing with Bmp280\n");
  printf("=================\n");
  EspConfig.init();
  const char *thingName = EspConfig.getThingName();
  Logger.init("Bmp280ThingTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("Bmp280ThingTest, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("Bmp280ThingTest, app_main()", "HttpServer started");
  EspTime.init();
  EspUdp.init();
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  EspMqttClient.init(thingName);
  Thing.init();
  Logger.info("ThingTest, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  Bmp280Sensor *sensorPtr = new Bmp280Sensor(SDA_GPIO, SCL_GPIO);
  IotSensor *temperatureSensorPtr =
      new Bmp280_Temperature(sensorPtr, thingName, "temperature", "Grad", 0.2, 9.9, 49.9);
  Thing.addSensor(temperatureSensorPtr);
  IotSensor *humiditySensorPtr =
      new Bmp280_Humidity(sensorPtr, thingName, "humidity", "%", 0.5, 0.0, 100.0);
  Thing.addSensor(humiditySensorPtr);
  IotSensor *pressureSensorPtr =
      new Bmp280_Pressure(sensorPtr, thingName, "pressure", "hPa", 0.5, 800.0, 1100.0);
  Thing.addSensor(pressureSensorPtr);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
