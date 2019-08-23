
#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>

#include <HX711.h>


const char *SERIAL_LOGGER_TAG = "SLT";

// HX711 circuit wiring
const gpio_num_t LOADCELL_DOUT_PIN = GPIO_NUM_23;
const gpio_num_t LOADCELL_SCK_PIN = GPIO_NUM_22;

// 2. Adjustment settings
const long LOADCELL_OFFSET = 50682624;
const long LOADCELL_DIVIDER = 5895655;

const float DIVIDER = -4.1; //-0.55;

extern "C"
{
  void app_main(void);
}

void app_main()
{
  // initArduino();
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("===============\n");
  printf("HX711-Test\n");
  printf("===============\n");
  EspConfig.init();
  Logger.init("HX711Test");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("HX711Test;app_main()", "Initialize HX711!");

  HX711 *hx711 = new HX711(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, "HX711", "weight", "g", 100);

  while (true)
  {
    hx711->measure();
    float weight = hx711->getLastMeasurement();
    sprintf(loggerMessage, "Weight:  %.1f", weight);
    Logger.info("HX711Test;app_main()", loggerMessage);
    vTaskDelay(5000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}
