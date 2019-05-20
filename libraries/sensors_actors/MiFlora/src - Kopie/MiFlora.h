#pragma once
#include <map>
#include <functional>
#include <cstring>
#include <BLEClient.h>
#include <BLEAddress.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson


struct StrCompare : public std::binary_function<const char*, const char*, bool> {
public:
    bool operator() (const char* str1, const char* str2) const
    { return std::strcmp(str1, str2) < 0; }
};

struct miflora_t
{
  // BLEAddress* bleAddress;
  char macAddress[20];
  long lastMeasurementTime;
  int rssi;
  int batteryLevel;
  int moisture;
  int brightness;
};

typedef std::map<const char *, miflora_t *, StrCompare> MiFloraMap;

struct cmp_str
{
   bool operator()(char const *a, char const *b) const
   {
      return strcmp(a, b) != 0;
   }
};

class MiFloraClass
{

public:
  void init();
  bool publishMiFloraSensorValues(miflora_t *miflora, BLEAddress bleAddress);

private:
  MiFloraMap* _miFloras;
  BLEClient*  _bleClient;
  StaticJsonBuffer<200> _jsonBuffer;
  void publishSensorValue(const char *macAddress, const char *sensorType, float value);

};

extern MiFloraClass MiFlora;
