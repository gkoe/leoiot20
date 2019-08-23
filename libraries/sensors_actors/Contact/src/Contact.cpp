#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"

#include "Contact.h"

bool getPinState(gpio_num_t pin)
{
  return gpio_input_get() & (1 << pin);
}

Contact::Contact(gpio_num_t pin, const char *thingName,
                 const char *name, const char *unit, float threshold) : IotSensor(thingName, name, unit, threshold)
{
  _pin = pin;
  gpio_pad_select_gpio(_pin);
  gpio_set_direction(_pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(_pin, GPIO_PULLUP_ONLY);
  //_isOn = IotSensor::getPinState(_pin);
}

void Contact::measure()
{
  bool isNowOn = IotSensor::getPinState(_pin);
  float value = 0.0;
  if (isNowOn)
  {
    value = 1.0;
  }
  else
  {
    value = 0.0;
  }
  setMeasurement(value);
}
