#include "HLW8012.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6; //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

static inline uint32_t get_time_us()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec;
}

#define timeout_expired(start, len) ((uint32_t)(get_time_us() - (start)) >= (len))

/* 
#define RETURN_CRTCAL(MUX, RES)  \
    do                           \
    {                            \
        portEXIT_CRITICAL(&MUX); \
        return RES;              \
    } while (0)
*/

#define TIMEOUT_1S 10000000

int HLW8012::measureCf1PulseDuration()
{
  // Wait for CF1 goes low
  uint32_t start = get_time_us();
  // Wait for HIGH
  while (!gpio_get_level(_pwmPin))
  {
    if (timeout_expired(start, TIMEOUT_1S))
      return -1;
  }
  // Wait for LOW
  start = get_time_us();
  while (gpio_get_level(_pwmPin))
  {
    if (timeout_expired(start, TIMEOUT_1S))
      return -2;
  }
  start = get_time_us();
  // Wait for HIGH
  while (!gpio_get_level(_pwmPin))
  {
    if (timeout_expired(start, TIMEOUT_1S))
      return -3;
  }
  return get_time_us() - start;
}

void HLW8012::measurePowerInBackground()
{
  int duration = measureCf1PulseDuration();
  if (duration < 0)
  {
    printf("!!! Error measuring duration, ErrorCode: %i\n", duration);
    _actPower = -1.0;
    return;
  }
  _lastMeasurements[_actIndex] = duration;
  _actPower = getAveragePower();
}

void measurePowerInLoopTask(void *pvParameter)
{
  HLW8012 *hlw8012 = (HLW8012 *)pvParameter;
  printf("MeasureTask started\n");

  while (1)
  {
    hlw8012->measurePowerInBackground();
    vTaskDelay(500 / portTICK_RATE_MS);
  }
}

HLW8012::HLW8012(gpio_num_t pwmPin, const char *thingName, const char *name, const char *unit, float threshold)
    : IotSensor(thingName, name, unit, threshold)
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(channel, atten);
  gpio_set_direction(pwmPin, GPIO_MODE_INPUT);
  _pwmPin = pwmPin;
  for (int i = 0; i < 10; i++)
  {
    _lastMeasurements[i] = -1;
  }
  xTaskCreate(measurePowerInLoopTask,   /* Task function. */
              "measurePowerInLoopTask", /* String with name of task. */
              4096,                     /* Stack size in words. */
              this,                     /* Parameter passed as input of the task */
              1,                        /* Priority of the task. */
              NULL                      /* Task handle. */
  );
}

float HLW8012::getAveragePower()
{
  int minValue = 1000;
  int maxValue = 0;
  int validValues = 0;
  int sumOfValues = 0;
  for (int i = 0; i < 10; i++)
  {
    if (_lastMeasurements[i] != -1)
    {
      uint32_t value = _lastMeasurements[i];
      if (value > maxValue)
      {
        maxValue = value;
      }
      else if (value < minValue)
      {
        minValue = value;
      }
      sumOfValues += value;
      validValues++;
    }
  }
  if (validValues < 3)
    return -1;
  printf("sumOfValues: %d, minValue: %d, maxValue: %d, validValues: %d\n", sumOfValues, minValue, maxValue, validValues);
  return (sumOfValues - minValue - maxValue) / ((float)validValues - 2);
}

/**
  measure() gets the measurment and set it.
*/
void HLW8012::measure()
{
  setMeasurement(_actPower);
}
