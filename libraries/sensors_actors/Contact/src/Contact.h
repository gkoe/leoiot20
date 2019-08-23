#include <IotSensor.h>


class Contact : public IotSensor
{
  public:
    Contact( gpio_num_t pin, const char *thingName, const char *name, 
                const char *unit, float threshold);
  	virtual void measure();

  private:
    gpio_num_t _pin;
};