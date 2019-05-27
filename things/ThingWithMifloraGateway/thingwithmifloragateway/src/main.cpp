#include <Arduino.h>

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspStation.h>
#include <HttpServer.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <EspMqttClient.h>
#include <SystemService.h>
#include <EspUdp.h>
#include <UdpLoggerTarget.h>
#include <Thing.h>

#define LED_BUILTIN_PIN 16 // WEMOS MINI32 2, TTGO 16, Lolin 5, sonst 21

const char *SERIAL_LOGGER_TAG = "SLT";
#define SLEEP_DURATION  720ll // duration of sleep between flora connection attempts in seconds (must be constant with "ll" suffix)
#define SLEEP_WAIT 90         // time until esp32 is put into deep sleep mode. must be sufficient to connect to wlan, connect to xiaomi flora device & push measurement data to MQTT


//>>>>>>>>>>>>>>>>>>>> Thingspezifisch
#include <MiFlora.h>
//<<<<<<<<<<<<<<<<<<<<<<<

void goToDeepSleep(){
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ll);
  Logger.info("MiFloraGateway;goToDeepSleep()", "Going to sleep now.");
  esp_deep_sleep_start();
}

void taskDeepSleep( void * parameter ){
  delay(SLEEP_WAIT * 1000);
  goToDeepSleep();
}

void setup()
{
	printf("==========================\n");
	printf("Thing with Miflora-Gateway\n");
	printf("==========================\n");
	EspConfig.init();
	const char *thingName = EspConfig.getThingName();
	Logger.init("MiFloraGateway");
	SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
	Logger.addLoggerTarget(serialLoggerTarget);
	SystemService.init();
	SystemService.heapSizeCanPushError(false); // BLE braucht viel zu viel Speicher
	SystemService.resetRestartsCounter();
	EspStation.init();
	Logger.info("MiFloraGateway, app_main()", "Waiting for connection!");
	while (!EspStation.isStationOnline())
	{
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
	HttpServer.init();
	Logger.info("MiFloraGateway, app_main()", "HttpServer started");
	EspTime.init();
	EspMqttClient.init(thingName);
	EspUdp.init();
	Thing.init();
	Logger.info("MiFloraGateway, app_main()", "Thing created");
	xTaskCreate(      taskDeepSleep,          /* Task function. */
                    "TaskDeepSleep",        /* String with name of task. */
                    4000,                  /* Stack size in words. */
                    NULL,                   /* Parameter passed as input of the task */
                    1,                      /* Priority of the task. */
                    NULL);                  /* Task handle. */
  
  	MiFlora.init();
	MiFlora.readNextMiFlora();
}

void loop()
{
	SystemService.checkSystem();
	Thing.refreshSensorsAndActors();
	vTaskDelay(1);
}