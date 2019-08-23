#include <Bmp280Sensor.h>

const gpio_num_t SDA_GPIO = GPIO_NUM_21;
const gpio_num_t SCL_GPIO = GPIO_NUM_22;

extern "C"
{
    void app_main(void);
}

void app_main()
{
    Bmp280Sensor *sensor = new Bmp280Sensor(SDA_GPIO, SCL_GPIO);

    while (true)
    {
        printf("Pressure: %.2f Pa, Temperature: %.2f °C, Humidity: %.2f %%\n", sensor->getPressure(), sensor->getTemperature(), sensor->getHumidity());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
