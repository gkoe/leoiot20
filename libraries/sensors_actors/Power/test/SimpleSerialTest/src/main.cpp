#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>

#include <Power.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>


const char *SERIAL_LOGGER_TAG = "SLT";

const adc1_channel_t adcChannel = ADC1_CHANNEL_6;  // GPIO34

extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("===============\n");
  printf("PowerSensor-Test\n");
  printf("===============\n");
  EspConfig.init();
  Logger.init("PowerSensorTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("PowerSensorTest;app_main()", "Initialize Powersensor!");

  Power *powerPtr = new Power(adcChannel, "powersensor", "power", "W", 2);

  while (true)
  {
    powerPtr->measure();
    float power = powerPtr->getLastMeasurement();
    sprintf(loggerMessage, "Power:  %.1f", power);
    Logger.info("PowerSensorTest;app_main()", loggerMessage);
    vTaskDelay(5000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}



