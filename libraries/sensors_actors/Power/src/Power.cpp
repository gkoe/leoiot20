#include "Power.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

const float CALIBRATION_DIVIDER = 2630.0;

/**
 * Liefert cirka 1000 Messwerte innerhalb von 40 ms (Zwei Perioden).
 * Führt zu ausreichender Genauigkeit
 */
long getSumOf1000MeasurementsIn40ms(adc1_channel_t adcChannel, int *maxValue)
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
    }
    return sum;
}

/**
 * Ermittelt alle Sekunden die aktuelle Leistung.
 */
void measurePowerInLoopTask(void *pvParameter)
{
    Power *powerPtr = (Power *)pvParameter;
    int maxValue = -1;
    while (1)
    {
        // long startTime = esp_timer_get_time() / 1000;
        long value = getSumOf1000MeasurementsIn40ms(powerPtr->_adcChannel, &maxValue);
        // long duration = esp_timer_get_time() / 1000 - startTime;
        powerPtr->_actPower = value / CALIBRATION_DIVIDER;
        // printf("!!! MaxValue: %i, power: %.1f\n", maxValue, powerPtr->_actPower);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

Power::Power(adc1_channel_t adcChannel, const char *thingName, const char *name, const char *unit, float threshold)
    : IotSensor(thingName, name, unit, threshold)
{
    _adcChannel = adcChannel;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adcChannel, ADC_ATTEN_DB_0);

    xTaskCreate(measurePowerInLoopTask,   /* Task function. */
                "measurePowerInLoopTask", /* String with name of task. */
                4096,                /* Stack size in words. */
                this,                /* Parameter passed as input of the task */
                1,                   /* Priority of the task. */
                NULL                 /* Task handle. */
    );
}

/**
  measure() gets the measurment and set it.
*/
void Power::measure()
{
    setMeasurement(_actPower);
}
