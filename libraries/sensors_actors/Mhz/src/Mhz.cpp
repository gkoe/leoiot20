#include "Mhz.h"
#include "driver/uart.h"

#include <Logger.h>
#include <SystemService.h>

#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE (100)

/*
  Alle 10 Sekunden CO2-Wert messen und in field speichern
*/
void mhzMeasurementInLoopTask(void *pvParameter)
{
  Mhz *mhzPtr = (Mhz *)pvParameter;

  while (1)
  {
    float value = mhzPtr->readCo2FromMhz();
    if (value >= 0)
    {
      mhzPtr->_ppmCo2 = value;
    }
    vTaskDelay(2000 / portTICK_RATE_MS);
  }
}

/**
  Mhz constructor takes the values and save these to lokale variables. Also makes a connection to the sensor.

  @param RX TX pins for the connection. 
         NodeName, name, unit and threshold needs the IotSensor constructor.
*/
Mhz::Mhz(gpio_num_t rxPin, gpio_num_t txPin, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
  uart_config_t uart_config = {
      .baud_rate = 9600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
      .use_ref_tick = false};
  uart_param_config(UART_NUM_1, &uart_config);
  uart_set_pin(UART_NUM_1, txPin, rxPin, UART_RTS, UART_CTS);
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
  // Configure a temporary buffer for the incoming data
  _data = (uint8_t *)malloc(BUF_SIZE);
  _lastMeasurementMilliSeconds = SystemService.getMillis();
  calibrate();
  xTaskCreate(mhzMeasurementInLoopTask,   /* Task function. */
              "mhzMeasurementInLoopTask", /* String with name of task. */
              4096,                       /* Stack size in words. */
              this,                       /* Parameter passed as input of the task */
              1,                          /* Priority of the task. */
              NULL                        /* Task handle. */
  );
}

/**
  calibrate() send the sensor some values and wait until it gets an OK.
*/
void Mhz::calibrate()
{
  Logger.info("Mhz;calibrate()", "Calibrating!");
  uart_write_bytes(UART_NUM_1, (const char *)_calibrate, 9);
  while (uart_read_bytes(UART_NUM_1, _data, BUF_SIZE, 5 / portTICK_RATE_MS) > 0)
  {
  }
}

/**
  getCheckSum() gets a packet and look if it is right and return a char

  @param packet a char pointer with the sendet message.
  @return a char with the checksum
*/
char Mhz::getCheckSum(char *packet)
{
  char checksum = 0x00;
  int i;
  for (i = 1; i < 8; i++)
  {

    checksum += packet[i];
  }
  checksum = 0xff - checksum;
  checksum += 1;
  return checksum;
}
float Mhz::getCo2()
{
  return _ppmCo2;
}

/** 
   readCo2FromMhz() reads the CO2 content, stores it in a private field for queries. If an error occurs, -1 is stored.
*/
float Mhz::readCo2FromMhz()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  float co2 = -1.0;
  uart_write_bytes(UART_NUM_1, (const char *)_cmd_measure_co2, 9);
  int len = uart_read_bytes(UART_NUM_1, _data, BUF_SIZE, 20 / portTICK_RATE_MS);
  if (len >= 9)
  {
    if (getCheckSum((char *)_data) == _data[8])
    {
      co2 += (uint16_t)_data[2] << 8;
      co2 += _data[3];
      // sprintf(loggerMessage, "Co2-Gehalt: %.1f", co2);
      // Logger.info("Mhz;readCo2FromMhz()", loggerMessage);
      if (co2 < 300 || co2 > 5000)
      {
        co2 = -1.0;
      }
    }
    else
    {
      sprintf(loggerMessage, "Checksumme stimmt nicht überein");
      Logger.error("Mhz;readCo2FromMhz()", loggerMessage);
      co2 = -1.0;
    }
  }
  else
  {
    sprintf(loggerMessage, "Statt 9 Zeichen nur %d Zeichen empfangen", len);
    Logger.error("Mhz;readCo2FromMhz()", loggerMessage);
    co2 = -1.0;
  }
  return co2;
}

/**
  measure() gets the measurment and set it.
*/
void Mhz::measure()
{
  setMeasurement(_ppmCo2);
}
