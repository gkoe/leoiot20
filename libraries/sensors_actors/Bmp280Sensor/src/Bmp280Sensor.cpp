#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <esp_system.h>
#include <string.h>

#include "Bmp280Sensor.h"
#include <Logger.h>
#include <Constants.h>

// static char TAG[] = "BMP280";

void measureBmp280InLoopTask(void *pvParameter)
{
    Bmp280Sensor *bmp280SensorPtr = (Bmp280Sensor *)pvParameter;

    while (1)
    {
        bmp280SensorPtr->readBmp280();
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

int Bmp280Sensor::readBmp280()
{
    if (bmp280_read_float(&_device, &_temperature, &_pressure, &_humidity) != ESP_OK)
    {
        printf("Temperature/pressure reading failed\n");
        return -1;
    }

    // printf("Pressure: %.2f Pa, Temperature: %.2f °C, Humidity: %.2f %%\n", _pressure, _temperature, _humidity);
    return 0;
}

Bmp280Sensor::Bmp280Sensor(gpio_num_t sdaPin, gpio_num_t sclPin)
{

    _sdaPin = sdaPin;
    _sclPin = sclPin;
    _humidity = -100;
    _temperature = -100;
    _pressure = -100;

    bmp280_params_t params;
    bmp280_init_default_params(&params);
    memset(&_device, 0, sizeof(bmp280_t));

    esp_err_t res;

    while (i2cdev_init() != ESP_OK)
    {
        printf("Could not init I2Cdev library\n");
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
    while (bmp280_init_desc(&_device, BMP280_I2C_ADDRESS_0, I2C_NUM_0, _sdaPin, _sclPin) != ESP_OK)
    {
        printf("Could not init device descriptor\n");
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    while ((res = bmp280_init(&_device, &params)) != ESP_OK)
    {
        printf("Could not init BMP280, err: %d\n", res);
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    bool bme280p = _device.id == BME280_CHIP_ID;
    printf("BMP280: found %s\n", bme280p ? "BME280" : "BMP280");
    xTaskCreatePinnedToCore(measureBmp280InLoopTask, "measureBmp280InLoopTask", configMINIMAL_STACK_SIZE * 8, this, 5, NULL, APP_CPU_NUM);

    // xTaskCreate(measureBmp280InLoopTask,   /* Task function. */
    //             "measureBmp280InLoopTask", /* String with name of task. */
    //             10000,                     /* Stack size in words. */
    //             this,                      /* Parameter passed as input of the task */
    //             1,                         /* Priority of the task. */
    //             NULL                       /* Task handle. */
    // );
}

float Bmp280Sensor::getPressure(){
    return _pressure/100.0;
}

float Bmp280Sensor::getTemperature(){
    return _temperature;
}

float Bmp280Sensor::getHumidity(){
    return _humidity;
}

