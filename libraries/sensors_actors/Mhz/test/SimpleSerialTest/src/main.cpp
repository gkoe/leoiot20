#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>

#include <Mhz.h>

const char *SERIAL_LOGGER_TAG = "SLT";
// #define UART_TXD (GPIO_NUM_12)
// #define UART_RXD (GPIO_NUM_13)
#define UART_TXD (GPIO_NUM_25)
#define UART_RXD (GPIO_NUM_33)

#define BUF_SIZE (1024)

extern "C"
{
  void app_main(void);
}

long getMillis()
{
  return esp_timer_get_time() / 1000;
}

bool getPinState(gpio_num_t pin)
{
  return gpio_input_get() & (1 << pin);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("========\n");
  printf("Mhz-Test\n");
  printf("========\n");
  EspConfig.init();
  Logger.init("ThingTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("MhzTest;app_main()", "Initialize Mhz!");

  Mhz *mhz = new Mhz(UART_RXD, UART_TXD, "Esp", "co2", "ppm", 5.0);

  while (true)
  {
    float ppm = mhz->getCo2();
    sprintf(loggerMessage, "Co2:  %.1f", ppm);
    Logger.info("MhzTest;app_main()", loggerMessage);
    vTaskDelay(5000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}
