# Library Mhz

## Ziele

Der ESP soll mit Hilfe dieser Libary die Werte von einen Mhz Sensor lesen.
Dieser Libary blockiert nicht den ESP dadurch gibt es keine Wartezeiten.

Diese Libary ist von der Libary Sensor abhängig.

## Hardware

Es wird lediglich ein ESP32 und ein Mhz Sensor benötigt.

## Software

### Beispielaufrufe

```c
#include <Arduino.h> 
#include <Mhz.h>


int Txpin = 12;
int Rxpin = 13;
 

Mhz _mhz(Rxpin,Txpin,"Esp","co2","ppm",5.0);


void setup() {
  Serial.begin(115200);
  Serial.println(F("MHZ-Test"));
  Serial.println(F("=========="));  
  delay(500);
}

void loop() {
  float ppm = _mhz.getCo2();
  Serial.print("PPM: ");
  Serial.println(ppm);
  delay(2000);
}
```

### Erklärung

* ```Mhz(int rxPin, int txPin, const char *nodeName, const char *name, const char *unit, float threshold)``` Muss als erstes aufgerufen werden. Darin werden die festlegen. 

### Library

##### Methoden

| ```Mhz::``` | Erklärung |
|-|-|
|```	Mhz(int rxPin, int txPin, const char* nodeName, const char* name, const char* unit, float threshold)```| Setzt die Pins beim Mhz Sensor. |
|```void calibrate()``` | Wird jedesmal vom Konstrukor aufgerufen. Dort wird der Mhz-Sensor kalibriert.  |
|``` char getCheckSum(char *packet)``` | Es wird mittels einer Formel eine Checksumme ausgerechnet.   |
|```float getCo2()``` | Gibt den letzten CO2-Messwert zurück.  |
|```virtual void measure()``` | Ruft die Methoden getCo2 und setMeasurement (abhängig von Sensor) auf und setzt die Werte |
