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
#include <Mhz.h>

#define UART_TXD (GPIO_NUM_12)  // verbunden mit Rx am Mhz
#define UART_RXD (GPIO_NUM_13)  // verbunden mit Tx am Mhz

// http://192.168.10.1/setconfig?ssid=SSID&password=PASSWORD
// http://192.168.10.1/setconfig?ntpserver=192.168.0.2
// http://192.168.10.1/setconfig?mqttbroker=192.168.0.122&mqttport=1883
// http://192.168.10.1/setconfig?thingname=co2demo 


extern "C"
{
  void app_main(void);
}

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("==============\n");
  printf("Thing with Mhz\n");
  printf("==============\n");
  Logger.init("MhzThing");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  EspConfig.init();
  SystemService.init();
  EspStation.init();
  Logger.info("MhzThing, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("MhzThing, app_main()", "HttpServer started");
  EspTime.init();
  EspUdp.init();
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  EspMqttClient.init(EspConfig.getThingName());
  Thing.init();
  Logger.info("MhzThing, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  Mhz *mhzPtr = new Mhz(UART_RXD, UART_TXD, "Esp", "co2", "ppm", 5.0, 300, 5000);
  Thing.addSensor(mhzPtr);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
