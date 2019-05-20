// esp32\libraries\sensors_actors\RgbLed\test\RgbLedTest

// P-Laufwerk: \\localhost\d$\....

#include <Sensor.h>
#include <RgbLed.h>

#define R 13
#define G 12
#define B 14

/****************************************** Statische Variablen **************************************/


RgbLed rgbLed(R, G, B, "RgbThing", "Test");

/****************************************** Setup **************************************/

void setup()
{
	Serial.begin(115200); //Initialisierung der seriellen Schnittstelle
	Serial.println();
	Serial.println("RgbLedTest");
	Serial.println("==========");
	Serial.println();
}

/****************************************** Loop **************************************/

void loop()
{
	Serial.println("Rot");
	rgbLed.setActor(rgbLed.Red);
	delay(2000);
	Serial.println("Grün");
	rgbLed.setActor(rgbLed.Green);
	delay(2000);
	Serial.println("Blau");
	rgbLed.setActor(rgbLed.Blue);
	delay(2000);
	Serial.println("Aus");
	rgbLed.setActor(rgbLed.Off);
	delay(2000);
	const char* color = "101010";
	Serial.println("Rot/Grün/Blau dunkel");
	rgbLed.setActor(color);
	delay(2000);
}
