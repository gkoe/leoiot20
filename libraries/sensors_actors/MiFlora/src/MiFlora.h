#pragma once
#include <list>
#include <functional>
#include <cstring>
#include <BLEClient.h>
#include <BLEAddress.h>

// const char* mifloraTopicsKey = "MIFLORA_TOPICS";


struct StrCompare : public std::binary_function<const char*, const char*, bool> {
public:
    bool operator() (const char* str1, const char* str2) const
    { return std::strcmp(str1, str2) < 0; }
};

/**
 * Macadresse und Feldstärke werden in einer Liste der gefundenen MiFloras sortiert nach 
 * Feldstärke gespeichert.
 */
struct miflora_t
{
  char macAddress[20];
  float rssiValue;
};

typedef std::list<miflora_t *> MiFloraList;

// struct cmp_str
// {
//    bool operator()(char const *a, char const *b) const
//    {
//       return strcmp(a, b) != 0;
//    }
// };

class MiFloraClass
{

public:
  void init();
  bool storeMiFloraSensorValuesToNvs(miflora_t *miflora);
  void closeBleConnection();
  void readNextMiFlora();

private:
  MiFloraList* _miFloras;
  BLEClient*  _bleClient;
  TaskHandle_t* _scanTask=nullptr;
  TaskHandle_t* _readTask=nullptr;
};

extern MiFloraClass MiFlora;
