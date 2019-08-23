#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <HLW8012.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>


const char *SERIAL_LOGGER_TAG = "SLT";

#define PWM_GPIO GPIO_NUM_13


extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("============\n");
  printf("HLW8012-Test\n");
  printf("============\n");
  EspConfig.init();
  Logger.init("HLW8012Test");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("HLW8012;app_main()", "Initialize HLW8012!");

  HLW8012 *hlw8012 = new HLW8012(PWM_GPIO, "HLW8012", "Power", "W", 1.0);

  while (true)
  {
    float power = hlw8012->getAveragePower();
    sprintf(loggerMessage, "Power:  %.1f", power);
    Logger.info("HLW8012Test;app_main()", loggerMessage);
    vTaskDelay(5000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}



