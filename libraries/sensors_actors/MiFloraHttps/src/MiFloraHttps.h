#pragma once
#include <list>
#include <functional>
#include <cstring>
#include <BLEClient.h>
#include <BLEAddress.h>

struct miflora_t
{
  char macAddress[20];
  float rssi;
};

class MiFloraHttpsClass
{

public:
  void init();
  void getAndSendMifloraValues();
  bool getNextMiflora();
  void closeBleConnection();

private:
  miflora_t _miflora;
  BLEClient *_bleClient;
  TaskHandle_t *_scanTask = nullptr;
};

extern MiFloraHttpsClass MiFloraHttps;
