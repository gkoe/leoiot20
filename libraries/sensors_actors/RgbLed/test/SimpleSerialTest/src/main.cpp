#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

gpio_num_t R = GPIO_NUM_13;
gpio_num_t G = GPIO_NUM_12;
gpio_num_t B = GPIO_NUM_14;

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
  int lastChangeMs;
  bool lastState;

  printf("==================\n");
  printf("Simple RgbLed-Test\n");
  printf("==================\n");
  gpio_pad_select_gpio(R);
  gpio_set_direction(R, GPIO_MODE_OUTPUT);
  gpio_pad_select_gpio(G);
  gpio_set_direction(G, GPIO_MODE_OUTPUT);
  gpio_pad_select_gpio(B);
  gpio_set_direction(B, GPIO_MODE_OUTPUT);
  gpio_set_level(R, 1);

  while (true)
  {
    vTaskDelay(1000 / (portTICK_PERIOD_MS));
    gpio_set_level(R, 0);
    gpio_set_level(G, 1);
    vTaskDelay(1000 / (portTICK_PERIOD_MS));
    gpio_set_level(G, 0);
    gpio_set_level(B, 1);
    vTaskDelay(1000 / (portTICK_PERIOD_MS));
    gpio_set_level(B, 0);
    gpio_set_level(R, 1);
  }
}
