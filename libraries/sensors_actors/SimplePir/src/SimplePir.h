#include <IotSensor.h>


class SimplePir : public IotSensor
{
  public:
    SimplePir( gpio_num_t pin, int interval, const char *thingName, const char *name, 
                const char *unit, float threshold);
  	virtual void measure();

  private:
    unsigned long  _lastMotionDetectedMs;
    gpio_num_t _pin;
    int _intervalMs; 
    bool _isMotion; 
};