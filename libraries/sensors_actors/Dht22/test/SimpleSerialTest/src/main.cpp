#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "Dht.h"

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspConfig.h>

static const char TAG[] = "DHT";

extern "C"
{
    void app_main(void);
}

void app_main()
{
    printf("==========\n");
    printf("Dht22 Test\n");
    printf("==========\n");
    EspConfig.init();
    Logger.init("Dht22Test");
    SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(TAG, LOG_LEVEL_VERBOSE);
    Logger.addLoggerTarget(serialLoggerTarget);

    Dht *dht = new Dht();
    dht->init(GPIO_NUM_27);
}