#pragma once
#include "esp_system.h"
#include <Constants.h>

/*
	Ein Sensor wird im System über den Thingnamen, der innerhalb des Things eindeutig ist)
	identifiziert.
	Der Sensor verwaltet auch die Einheit seines Messwertes und die Änderungsschwelle,
	die erreicht werden muss, um eine neue Meldung des Messwertes auszulösen.

	Sensor ist eine abstrakte Klasse. Die konkreten Sensorklassen müssen nur mehr die
	Methode measure() überschreiben, mit deren Hilfe der konkrete Code zum Einlesen 
	der Messwerte implementiert wird.
*/
class IotSensor
{

	const int MAX_INTERVALL = 900;
	const int MIN_ILLEGAL_VALUE_TIMESPAN = 5000;

public:
	/*
		Sensor wird mit seinem Namen, einer innerhalb des Things eindeutigen id,
		der Einheit des Messwertes und der Schwelle, ab der eine Änderung des 
		Messwertes gemeldet wird.
	*/
	IotSensor(const char *thingName, const char *name, const char *unit, float threshold, float minValue = -9999.9, float maxValue = 9999.9, bool getAverageValue = true);

	/*
		Liefert den letzten gemessenen Messwert. Dieser muss nicht mit dem 
		aktuellen Messwert übereinstimmen, da sich der aktuelle Wert innerhalb
		der Schwelle geändert haben kann und daher noch nicht übermittelt wurde.
	*/
	float getLastMeasurement();

	float getAverageValue();

	/*
		Aktueller Messwert wird eingetragen. Löst bei ausreichender Änderung
		Notification aus.
	*/
	void setMeasurement(float value);

	/*
		Liefert den Namen des Sensors
	*/
	char *getName();

	char *getUnit();

	void setMaxIntervall(int intervall);

	/*
		Abstrakte Messmethode muss vom konkreten Sensor überschrieben (implementiert)
		werden.
	*/
	virtual void measure(); // abstrakte Sensoren können nicht in Liste verwaltet werden
	// virtual void measure()=0; // abstrakte Sensoren können nicht in Liste verwaltet werden

	virtual void getMqttPayload(char *payload, float measurement); //! wozu der char* als Rückgabetyp

	/// statische Hilfsmethoden

	static bool getPinState(gpio_num_t pin);

protected:
private:
	long _lastIllegalValueTime;
	char _thingName[LENGTH_THING_NAME];
	char _name[LENGTH_MIDDLE_TEXT];
	char _unit[LENGTH_SHORT_TEXT];
	float _threshold;
	int _maxIntervall;
	float _minValue;
	float _maxValue;
	float _publishedMeasurement = 0.0; // letzter übertragener Wert
	float _lastMeasurement = 0.0;	  // letzter gemessener Wert
	long _time;
	uint32_t _lastValues[10];
	int _actLastValuesIndex = 0;
	bool _getAverageValue;
};
