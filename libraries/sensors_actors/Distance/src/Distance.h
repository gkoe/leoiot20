/**
 * @file ultrasonic.h
 *
 * ESP-IDF driver for ultrasonic range meters, e.g. HC-SR04, HY-SRF05 and so on
 *
 * Ported from esp-open-rtos
 * Copyright (C) 2016, 2018 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 */
#pragma once
#include "driver/gpio.h"
#include <IotSensor.h>
#include <driver/gpio.h>

#define ESP_ERR_ULTRASONIC_PING 0x200
#define ESP_ERR_ULTRASONIC_PING_TIMEOUT 0x201
#define ESP_ERR_ULTRASONIC_ECHO_TIMEOUT 0x202

#define MAX_DISTANCE 200

class Distance : public IotSensor
{
public:
    Distance(gpio_num_t triggerPin, gpio_num_t echoPin, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue);
    float readDistance();

    /**
    * Measure distance
    * \param dev Pointer to the device descriptor
    * \param max_distance Maximal distance to measure, centimeters
    * \return Distance in centimeters or ULTRASONIC_ERROR_xxx if error occured
    */
    esp_err_t measureDistance(uint32_t *distance);

    // float getAverageDistance();
    void setActDistance(uint32_t distance);

private:
    uint8_t *_data;
    gpio_num_t _triggerPin;
    gpio_num_t _echoPin;

    volatile float _actDistance = -1;

    virtual void measure();
};
