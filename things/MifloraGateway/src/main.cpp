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
#include <MiFlora.h>

extern "C"
{
  void app_main(void);
}

const gpio_num_t DHT22_PIN = GPIO_NUM_27;

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("===============\n");
  printf("MiFlora Gateway\n");
  printf("===============\n");
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
  MiFlora.init();
  Logger.info("MiFloraGateway, app_main()", "MiFlora init");

  long nextMifloraScan=SystemService.getMillis();
  uint32_t heapBefore;
  uint32_t heapAfter;
  int loopRound = 0;
  while (true)
  {
    SystemService.checkSystem();
    if (SystemService.getMillis() > nextMifloraScan)
    {
      loopRound++;
      MiFlora.doLoopMiFloras();
      nextMifloraScan = SystemService.getMillis() + 30000;
    }

    vTaskDelay(1);
  }
}
