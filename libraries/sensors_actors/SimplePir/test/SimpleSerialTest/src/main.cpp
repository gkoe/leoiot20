#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

gpio_num_t PIR_PIN = GPIO_NUM_26;


extern "C"
{
  void app_main(void);
}

long getMillis()
{
  return esp_timer_get_time() / 1000;
}

bool getPinState(gpio_num_t pin){
  return gpio_input_get() & (1<<pin);
}

void app_main()
{
  int lastChangeMs;
  bool lastState;

  printf("========\n");
  printf("PIR-Test\n");
  printf("========\n");
  gpio_pad_select_gpio(PIR_PIN);
  gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);
  lastChangeMs = getMillis();
  lastState = getPinState(PIR_PIN);

  while (true)
  {
    bool state = getPinState(PIR_PIN);
    if (state != lastState)
    {
      int duration = getMillis() - lastChangeMs;
      printf("State changed from %i to %i after %dms\n", lastState, state, duration);
      lastChangeMs = getMillis();
      lastState = state;
    }
    vTaskDelay(1);
  }
}
