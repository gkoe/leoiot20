#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <driver/adc.h>

const float CALIBRATION_DIVIDER = 2900.0;
const adc1_channel_t adcChannel = ADC1_CHANNEL_6; // GPIO34

extern "C"
{
  void app_main(void);
}

// /**
//  * Ermittelt den Maximalwert innerhalb der Periode
//  * Liefert ungenaue Ergebnisse. Besser funktioniert
//  * die Aufintegration der gesamten Messwerte
//  */
// int findMaxInOnePeriode()
// {
//   long start = esp_timer_get_time();
//   int value;
//   int maxValue = 0;
//   int samples = 0;
//   while (esp_timer_get_time() - start < 200000) // 20 ms lang abtasten
//   {
//     value = adc1_get_raw(ADC1_CHANNEL_6);
//     samples++;
//     // printf("          Value: %i\n", value);
//     if (value > maxValue)
//     {
//       maxValue = value;
//     }
//     // vTaskDelay(1);
//   }
//   printf("Samples: %i, ", samples);
//   return maxValue;
// }

/**
 * Liefert cirka 1000 Messwerte innerhalb von 40 ms (Zwei Perioden).
 * Führt zu ausreichender Genauigkeit
 */
long getSumOf1000MeasurementsIn40ms(int *maxValue)
{
  *maxValue = -1;
  int value;
  int samples = 0;
  long sum = 0;
  for (int i = 0; i < 1000; i++) // 5000
  {
    value = adc1_get_raw(adcChannel);
    if (value > *maxValue)
    {
      *maxValue = value;
    }
    samples++;
    sum += value;
    // vTaskDelay(1);
  }
  printf("Samples: %i, ", samples);
  return sum;
}

/**
 * Ermittelt alle Sekunden die aktuelle Leistung.
 */
void measureInLoopTask(void *pvParameter)
{
  int maxValue = -1;
  while (1)
  {
    long startTime = esp_timer_get_time() / 1000;
    long value = getSumOf1000MeasurementsIn40ms(&maxValue);
    int duration = esp_timer_get_time() / 1000 - startTime;
    float power = value / CALIBRATION_DIVIDER;
    printf("MaxValue: %i, Duration: %i, power: %.1f\n", maxValue, duration, power);
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
}

void app_main()
{
  // int lastChangeMs;
  // bool lastState;

  printf("========\n");
  printf("ADC-Test\n");
  printf("========\n");

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);

  xTaskCreate(measureInLoopTask,   /* Task function. */
              "measureInLoopTask", /* String with name of task. */
              4096,                /* Stack size in words. */
              NULL,                /* Parameter passed as input of the task */
              1,                   /* Priority of the task. */
              NULL                 /* Task handle. */
  );
  char buffer[300];
  while (true)
  {
    long millis = esp_timer_get_time() / 1000;
    sprintf(buffer, "Mainloop: %ld", millis);
    // printf("                                  %s\n", buffer);
    vTaskDelay(1);
  }
}
