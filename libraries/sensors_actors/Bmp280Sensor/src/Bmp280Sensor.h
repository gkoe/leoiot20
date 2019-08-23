#pragma once
#include "driver/gpio.h"

#include <bmp280.h>

class Bmp280Sensor 
{
public:
    Bmp280Sensor(gpio_num_t sdaPin, gpio_num_t sclPin);

    gpio_num_t _sdaPin = GPIO_NUM_21;
    gpio_num_t _sclPin = GPIO_NUM_22;
	int readBmp280();
	float getHumidity();
	float getTemperature();
	float getPressure();

private:
    bmp280_t _device;
	float _humidity = 0.0;
	float _temperature = 0.0;
	float _pressure = 0.0;

};