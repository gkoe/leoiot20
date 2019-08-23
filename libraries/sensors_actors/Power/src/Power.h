#pragma once
#include "driver/gpio.h"
#include <IotSensor.h>
#include <driver/adc.h>

class Power : public IotSensor
{
public:
    Power(adc1_channel_t adcChannel, const char *thingName, const char *name, const char *unit, float threshold);

    // /**
    // * Measure distance
    // * \param dev Pointer to the device descriptor
    // * \param max_distance Maximal distance to measure, centimeters
    // * \return Distance in centimeters or ULTRASONIC_ERROR_xxx if error occured
    // */
    // esp_err_t measureDistance(uint32_t *distance);

    // float getAverageDistance();
    // void setNextDistance(uint32_t distance);

    adc1_channel_t _adcChannel;
    volatile float _actPower = -1;
    virtual void measure();

private:

};
