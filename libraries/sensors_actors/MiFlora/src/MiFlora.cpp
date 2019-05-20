#include <MiFlora.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <config.h>

#include <EspConfig.h>
#include <EspTime.h>
#include <Logger.h>
#include <Constants.h>
#include <IotSensor.h>

// #include <sstream>

#define SCAN_TIME 10 // seconds

#define btoa(x) ((x) ? "true" : "false")

// class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
// {
//     void onResult(BLEAdvertisedDevice advertisedDevice)
//     {
//         Serial.printf("!!! Advertised Device: %s \n", advertisedDevice.toString().c_str());
//     }
// };
// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");
static BLEUUID miFloraUUID("0000fe95-0000-1000-8000-00805f9b34fb");

//const char *miFloraUUID = "0000fe95-0000-1000-8000-00805f9b34fb";
// The characteristic of the remote service we are interested in.
static BLEUUID uuid_version_battery("00001a02-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");

static BLERemoteCharacteristic *pRemoteCharacteristic;
//static BLEScanResults foundDevices;

bool MiFloraClass::setMiFloraSensorValues(miflora_t *miFlora)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];

    sprintf(loggerMessage, "Forming a connection to Flora device at %s", miFlora->macAddress);
    Logger.info("MiFlora, setMiFloraSensorValues()", loggerMessage);
    // Connect to the remove BLE Server.
    BLEAddress bleAddress = BLEAddress(std::string(miFlora->macAddress));
    if (!_bleClient->connect(bleAddress))
    {
        sprintf(loggerMessage, "No connection to: %s", miFlora->macAddress);
        Logger.info("MiFlora, setMiFloraSensorValues()", loggerMessage);
        return false;
    }
    sprintf(loggerMessage, "Connected with: %s", miFlora->macAddress);
    Logger.info("MiFlora, setMiFloraSensorValues()", loggerMessage);

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = _bleClient->getService(serviceUUID);
    if (pRemoteService == nullptr)
    {
        Logger.error("MiFlora, setMiFloraSensorValues()", "Failed to find our service UUID");
        return false;
    }
    Logger.info("MiFlora, setMiFloraSensorValues()", "Found our service");

    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
    uint8_t buf[2] = {0xA0, 0x1F};
    pRemoteCharacteristic->writeValue(buf, 2, true);

    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
    //Serial.println(pRemoteService->toString().c_str());
    if (pRemoteCharacteristic == nullptr)
    {
        Logger.error("MiFlora, setMiFloraSensorValues()", "Failed to find our characteristic UUID");
        closeBleConnection();
        return false;
    }
    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    sprintf(loggerMessage, "Found our characteristic value was: %s", value);
    Logger.info("MiFlora, setMiFloraSensorValues()", loggerMessage);

    const char *val = value.c_str();

    // Serial.print("Hex: ");
    // for (int i = 0; i < 16; i++)
    // {
    //     Serial.print((int)val[i], HEX);
    //     Serial.print(" ");
    // }
    // Serial.println(" ");

    float temperature = (val[0] + val[1] * 256) / ((float)10.0);
    float moisture = (float)val[7];
    int brightness = val[3] + (float)(val[4] * 256);
    int conductivity = (float)(val[8] + val[9] * 256);
    printf("!!! set moisture %f\n", moisture);
    miFlora->moistureSensor->setMeasurement(moisture);
    printf("!!! set temperature %f\n", temperature);
    miFlora->temperatureSensor->setMeasurement(temperature);
    printf("!!! set brightness %d\n", brightness);
    miFlora->brightnessSensor->setMeasurement(brightness);
    printf("!!! set conductivity %d\n", conductivity);
    miFlora->conductivitySensor->setMeasurement(conductivity);

    // Serial.println("!!! Trying to retrieve battery level...");
    // pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_version_battery);
    // if (pRemoteCharacteristic == nullptr)
    // {
    //     Serial.println("!!! Failed to find our characteristic UUID");
    //     //Serial.println(uuid_sensor_data.toString().c_str());
    //     _bleClient->disconnect();
    //     return false;
    // }
    // Serial.println("!!!Found our characteristic");

    // // Read the value of the characteristic...
    // value = pRemoteCharacteristic->readValue();
    // Serial.print("!!! The characteristic value was: ");
    // const char *val2 = value.c_str();
    // Serial.print("Hex: ");
    // for (int i = 0; i < 16; i++)
    // {
    //     Serial.print((int)val2[i], HEX);
    //     Serial.print(" ");
    // }
    // Serial.println(" ");

    // float batteryLevel = (float)val2[0];
    // miFlora->batteryLevelSensor->setMeasurement(batteryLevel);

    return true;
}

void MiFloraClass::closeBleConnection()
{
    if (_bleClient->isConnected())
        _bleClient->disconnect();
    // _bleClient->~BLEClient();
    // BLEDevice::deinit(true);
}

void readValuesFromMiFloraTask(void *voidPtr)
{
    miflora_t *miFlora = (miflora_t *)voidPtr;
    printf("!!! readValuesFromMiFloraTask from %s\n", miFlora->macAddress);
    MiFlora.setMiFloraSensorValues(miFlora);
    MiFlora.closeBleConnection();
    vTaskDelete(NULL);
}

void scanMiFlorasTask(void *voidPtr)
{
    printf("!!! scanMiFlorasTask, begin\n");
    MiFloraMap *miFloras = (MiFloraMap *)voidPtr;
    // Serial.printf("!!! miFloras in search, Count: %d\n", miFloras->size());
    BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
    // pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(0x50);
    pBLEScan->setWindow(0x30);
    printf("!!! Start BLE scan for %d seconds...\n", SCAN_TIME);
    BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
    int count = foundDevices.getCount();
    printf("!!! Found BLE-devices: %d\n", count);
    // int newMiFlorasCounter = 0;
    // for (int idx = 0; idx < count; idx++)
    // {
    //     BLEAdvertisedDevice device = foundDevices.getDevice(idx);
    //     BLEUUID serviceUUID = device.getServiceUUID();
    //     char name[LENGTH_TOPIC];
    //     // Serial.printf("!!! device-uuid %s: \n",serviceUUID.toString().c_str());
    //     if (serviceUUID.equals(miFloraUUID)) // Ist der BLE-Server ein MiFlora?
    //     {
    //         // Serial.printf("!!! device %d is a miflora\n",idx);
    //         char macAddress[20];
    //         // //esp_bd_addr_t* adrP = device.getAddress().getNative();
    //         // //sprintf(macAddress, "%x:%x:%x:%x:%x:%x", *adrP[0], *adrP[1], *adrP[2], *adrP[3], *adrP[4], *adrP[5]);
    //         strcpy(macAddress, device.getAddress().toString().c_str());
    //         int rssi = device.getRSSI();
    //         Serial.printf("!!! MiFlora %d, address: %s, rssi: %d\n", idx, macAddress, rssi);
    //         miflora_t *miFlora;
    //         if (miFloras->find(macAddress) == miFloras->end())
    //         { // MiFlora ist nicht in Map
    //             // Serial.println("!!! MiFlora not in map");
    //             miFlora = new miflora_t();
    //             strcpy(miFlora->macAddress, macAddress);
    //             sprintf(name, "%s/%s", macAddress, "moisture");
    //             IotSensor *moistureSensor = new IotSensor(Thing.getName(), name, "%", 1.0, 36000);
    //             Thing.addSensor(moistureSensor);
    //             miFlora->moistureSensor = moistureSensor;
    //             sprintf(name, "%s/%s", macAddress, "temperature");
    //             IotSensor *temperatureSensor = new IotSensor(Thing.getName(), name, "", 2.0, 36000);
    //             Thing.addSensor(temperatureSensor);
    //             miFlora->temperatureSensor = temperatureSensor;
    //             sprintf(name, "%s/%s", macAddress, "brightness");
    //             IotSensor *brightnessSensor = new IotSensor(Thing.getName(), name, "", 10.0, 36000);
    //             Thing.addSensor(brightnessSensor);
    //             miFlora->brightnessSensor = brightnessSensor;
    //             sprintf(name, "%s/%s", macAddress, "conductivity");
    //             IotSensor *conductivitySensor = new IotSensor(Thing.getName(), name, "", 10.0, 36000);
    //             Thing.addSensor(conductivitySensor);
    //             miFlora->conductivitySensor = conductivitySensor;
    //             sprintf(name, "%s/%s", macAddress, "batterylevel");
    //             IotSensor *batteryLevelSensor = new IotSensor(Thing.getName(), name, "", 10.0, 36000);
    //             Thing.addSensor(batteryLevelSensor);
    //             miFlora->batteryLevelSensor = batteryLevelSensor;
    //             sprintf(name, "%s/%s", macAddress, "rssi");
    //             IotSensor *rssiSensor = new IotSensor(Thing.getName(), name, "", 10.0, 36000);
    //             Thing.addSensor(rssiSensor);
    //             miFlora->rssiSensor = rssiSensor;
    //             miFloras->insert({miFlora->macAddress, miFlora});
    //             newMiFlorasCounter++;
    //             // Serial.printf("!!! New MiFlora, MAC: %s\n", miFlora->macAddress);
    //         }
    //         else
    //         { // MiFlora ist in Map
    //             // Serial.println("!!! MiFlora in map");
    //             miFlora = miFloras->at(macAddress);
    //         }
    //     }
    // }
    // // pBLEScan->clearResults();
    // // Serial.println("!!! Scan done!");
    // int miFlorasCounter = miFloras->size();
    // Serial.printf("!!! Scanned mifloras: %d, new: %d\n", miFlorasCounter, newMiFlorasCounter);
    MiFlora.closeBleConnection();
    vTaskDelete(NULL);
}

/**
 * Die erreichbaren MiFloras werden im ersten Schritt gescannt (mal sehen, welche MiFloras es gibt)
 * In den weiteren Aufrufen wird ein MiFlora nach dem anderen abgefragt.
 */
void MiFloraClass::doLoopMiFloras()
{
    //BLEDevice::init("");
    //_bleClient = BLEDevice::createClient();
    printf("!!! doLoopMiFloras(), begin\n");
    if (_bleClient->isConnected())
    {
        printf("??? BLE-Client was connected ==> disconnect!\n");
        closeBleConnection();
        return;
    }

    printf("!!! delete old tasks\n");
    if (_scanTask != nullptr)
        vTaskDelete(_scanTask);
    if (_readTask != nullptr)
        vTaskDelete(_readTask);

    if (_actMiFloraIndex > _miFloras->size())
    { // 0 steht für MiFloras neu einlesen, 1-Lenght für den jeweiligen MiFlora
        _actMiFloraIndex = 0;
    }
    if (_actMiFloraIndex == 0) // Scannen der MiFloras starten
    {
        xTaskCreatePinnedToCore(scanMiFlorasTask,   /* Task function. */
                                "TaskScanMiFloras", /* String with name of task. */
                                10000,              /* Stack size in words. */
                                _miFloras,          /* Parameter passed as input of the task */
                                1,                  /* Priority of the task. */
                                _scanTask,          /* Task handle. */
                                0                   /* Core */
        );
    }
    else
    {
        if (_miFloras->size() == 0)
        {
            closeBleConnection();
            return;
        }

        MiFloraMap::iterator it = _miFloras->begin();
        int index = _actMiFloraIndex - 1;
        printf("!!! Read values from miflora: %d\n", index);
        while (index > 0 && it != _miFloras->end())
        {
            it++;
            index--;
        }
        if (it == _miFloras->end())
        {
            closeBleConnection();
            return;
        }
        miflora_t *miFlora = it->second;
        xTaskCreatePinnedToCore(readValuesFromMiFloraTask,   /* Task function. */
                                "readValuesFromMiFloraTask", /* String with name of task. */
                                10000,                       /* Stack size in words. */
                                miFlora,                     /* Parameter passed as input of the task */
                                1,                           /* Priority of the task. */
                                _readTask,                   /* Task handle. */
                                0                            /* Core */
        );
    }
    //!!! _actMiFloraIndex++;
}

void MiFloraClass::init()
{
    _miFloras = new MiFloraMap();
    BLEDevice::init("");
    _bleClient = BLEDevice::createClient();
}

MiFloraClass MiFlora;
