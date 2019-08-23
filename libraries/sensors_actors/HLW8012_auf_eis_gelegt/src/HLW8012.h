#include <IotSensor.h>

class HLW8012 : public IotSensor
{
public:
    HLW8012(gpio_num_t pwmPin, const char *thingName, const char *name, const char *unit, float threshold);

    float readPower();
    void measurePowerInBackground();
    int measureCf1PulseDuration();

    esp_err_t measureDistance(uint32_t *distance);

    float getAveragePower();
    void setActPower(float power);

private:
    gpio_num_t _pwmPin;

    uint32_t _lastMeasurements[10];
    int _actIndex = 0;

    volatile float _actPower = -1.0;

    virtual void measure();
};
