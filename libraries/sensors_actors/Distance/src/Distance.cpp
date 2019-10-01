/**
 * @file ultrasonic.c
 *
 * ESP-IDF driver for ultrasonic range meters, e.g. HC-SR04, HY-SRF05 and so on
 *
 * Ported from esp-open-rtos
 * Copyright (C) 2016, 2018 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 */
#include "Distance.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include <Logger.h>

#define TRIGGER_LOW_DELAY 4
#define TRIGGER_HIGH_DELAY 10
#define PING_TIMEOUT 6000
#define ROUNDTRIP 58

//static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

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

void measureDistanceInLoopTask(void *pvParameter)
{
    Distance *distancePtr = (Distance *)pvParameter;
    uint32_t distance;
    esp_err_t errorCode;

    while (1)
    {
        errorCode = distancePtr->measureDistance(&distance);
        if (errorCode == ESP_OK)
        {
            distancePtr->setActDistance(distance);
            // printf("Distance measured: %d\n", distance);
        }
        else
        {
            switch (errorCode)
            {
            case ESP_ERR_ULTRASONIC_PING:
                printf("!!! Error reading ultrasonic, Cannot ping (device is in invalid state)\n");
                Logger.error("ultrasonic;measureDistanceInLoopTask()", "Cannot ping (device is in invalid state)");
                break;
            case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                //printf("!!! Error reading ultrasonic, Ping timeout (no device found)\n");
                break;
            case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:

                printf("!!! Error reading ultrasonic, Echo timeout (i.e. distance too big)\n");
                Logger.error("ultrasonic;measureDistanceInLoopTask()", "Echo timeout (i.e. distance too big)");
                break;
            default:
                printf("%d\n", errorCode);
            }
        }
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

Distance::Distance(gpio_num_t triggerPin, gpio_num_t echoPin, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
    gpio_set_direction(triggerPin, GPIO_MODE_OUTPUT);
    gpio_set_direction(echoPin, GPIO_MODE_INPUT);
    gpio_set_level(triggerPin, 0);
    _triggerPin = triggerPin;
    _echoPin = echoPin;
    xTaskCreate(measureDistanceInLoopTask,   /* Task function. */
                "measureDistanceInLoopTask", /* String with name of task. */
                4096,                        /* Stack size in words. */
                this,                        /* Parameter passed as input of the task */
                1,                           /* Priority of the task. */
                NULL                         /* Task handle. */
    );
}

void Distance::setActDistance(uint32_t distance)
{
    _actDistance = distance;
}

esp_err_t Distance::measureDistance(uint32_t *distance)
{

    // portENTER_CRITICAL(&mux);

    // Ping: Low for 2..4 us, then high 10 us
    gpio_set_level(_triggerPin, 0);
    ets_delay_us(TRIGGER_LOW_DELAY);
    gpio_set_level(_triggerPin, 1);
    ets_delay_us(TRIGGER_HIGH_DELAY);
    gpio_set_level(_triggerPin, 0);

    // Previous ping isn't ended
    if (gpio_get_level(_echoPin))
        return ESP_ERR_ULTRASONIC_PING;
    // RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING);

    // Wait for echo
    uint32_t start = get_time_us();
    while (!gpio_get_level(_echoPin))
    {
        if (timeout_expired(start, PING_TIMEOUT))
            return ESP_ERR_ULTRASONIC_PING_TIMEOUT;
        // RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING_TIMEOUT);
    }

    // got echo, measuring
    uint32_t echo_start = get_time_us();
    uint32_t time = echo_start;
    uint32_t meas_timeout = echo_start + (MAX_DISTANCE * ROUNDTRIP);
    while (gpio_get_level(_echoPin))
    {
        time = get_time_us();
        if (timeout_expired(echo_start, meas_timeout))
            return ESP_ERR_ULTRASONIC_ECHO_TIMEOUT;
            // RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_ECHO_TIMEOUT);
    }
    // portEXIT_CRITICAL(&mux);

    *distance = (time - echo_start) / ROUNDTRIP;

    return ESP_OK;
}

/**
  measure() gets the measurment and set it.
*/
void Distance::measure()
{
    setMeasurement(_actDistance);
}
