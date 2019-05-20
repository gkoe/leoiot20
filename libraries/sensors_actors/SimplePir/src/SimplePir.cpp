#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"

#include "SimplePir.h"

bool getPinState(gpio_num_t pin){
  return gpio_input_get() & (1<<pin);
}

long getMillis()
{
  return esp_timer_get_time() / 1000;
}


SimplePir::SimplePir(gpio_num_t pin, int interval, const char *thingName, 
    const char *name, const char *unit, float threshold) : IotSensor(thingName, name, unit, threshold)
{
  _pin = pin;
  gpio_pad_select_gpio(_pin);
  gpio_set_direction(_pin, GPIO_MODE_INPUT);
  _isMotion = getPinState(_pin);
  _lastMotionDetectedMs = getMillis();
  _intervalMs = interval * 1000;
}

void SimplePir::measure()
{
  bool isNowMotion = getPinState(_pin);
  if (!isNowMotion && !_isMotion) {  // keine Änderung
    return;
  }
  if (isNowMotion == true ) {  // neue Bewegung erkannt
    _lastMotionDetectedMs = getMillis();  // Delay gilt ab jetzt
    if (_isMotion == false) {
      _isMotion=true;
      setMeasurement(1.0);
    }
  }
  else{  // isNowMotion == false && _pirState == true  ==> Zeitablauf
    if (getMillis() > _lastMotionDetectedMs + _intervalMs) {
      _isMotion = false;
      setMeasurement(0.0);
    }
  }
}
