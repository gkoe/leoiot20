#include <MiFlora.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <config.h>
#include <rom/rtc.h>

#include <EspConfig.h>
#include <EspTime.h>
#include <Logger.h>
#include <Constants.h>
// #include <IotSensor.h>
// #include <Thing.h>
#include <SystemService.h>

// #include <sstream>

#define SCAN_TIME 10 // seconds

#define btoa(x) ((x) ? "true" : "false")

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");
static BLEUUID miFloraUUID("0000fe95-0000-1000-8000-00805f9b34fb");

//const char *miFloraUUID = "0000fe95-0000-1000-8000-00805f9b34fb";
// The characteristic of the remote service we are interested in.
static BLEUUID uuid_version_battery("00001a02-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");

static BLERemoteCharacteristic *pRemoteCharacteristic;

static void setNvsStringIfValid(float value, const char *name, float min, float max)
{
    char valueText[LENGTH_SHORT_TEXT];

    if (value >= min && value <= max)
    {
        sprintf(valueText, "%.2f", value);
        EspConfig.setNvsStringValue(name, valueText);
    }
    else
    {
        EspConfig.setNvsStringValue(name, "");
    }
}

/**
 * Einen Miflira auslesen und die Messwerte in NVS speichern
 */
bool MiFloraClass::storeMiFloraSensorValuesToNvs(miflora_t *miFlora)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    // Connect to the remove BLE Server.
    BLEAddress bleAddress = BLEAddress(std::string(miFlora->macAddress));
    if (!_bleClient->connect(bleAddress))
    {
        snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "No connection to: %s", miFlora->macAddress);
        Logger.error("MiFlora;setMiFloraSensorValues()", loggerMessage);
        return false;
    }
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Connected with: %s", miFlora->macAddress);
    Logger.info("MiFlora;setMiFloraSensorValues()", loggerMessage);

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = _bleClient->getService(serviceUUID);
    if (pRemoteService == nullptr)
    {
        Logger.error("MiFlora;setMiFloraSensorValues()", "Failed to find our service UUID");
        return false;
    }
    // Logger.info("MiFlora;setMiFloraSensorValues()", "Found our service");

    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
    uint8_t buf[2] = {0xA0, 0x1F};
    pRemoteCharacteristic->writeValue(buf, 2, true);
    Logger.info("MiFlora;setMiFloraSensorValues()", "written A0 1F to characteristics");

    vTaskDelay(1000 / portTICK_PERIOD_MS); //! war 500

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
    //Serial.println(pRemoteService->toString().c_str());
    if (pRemoteCharacteristic == nullptr)
    {
        Logger.error("MiFlora;setMiFloraSensorValues()", "Failed to find our characteristic UUID");
        closeBleConnection();
        return false;
    }
    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Found our characteristic UUID");
    Logger.info("MiFlora;setMiFloraSensorValues()", loggerMessage);

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
    // // printf("!!! set moisture %f\n", moisture);
    // // miFlora->moistureSensor->setMeasurement(moisture);
    Logger.info("MiFlora;setMiFloraSensorValues();mac:", miFlora->macAddress);
    EspConfig.setNvsStringValue("mac", miFlora->macAddress);

    setNvsStringIfValid(moisture, "moisture", 0.0, 100.0);
    setNvsStringIfValid(temperature, "temperature", 0.0, 40.0);
    setNvsStringIfValid(brightness, "brightness", 0.0, 10000.0);
    setNvsStringIfValid(conductivity, "conductivity", 0.0, 1000.0);
    setNvsStringIfValid(miFlora->rssiValue, "rssi", -200.0, 100.0);

    Logger.info("MiFlora;setMiFloraSensorValues()", "Trying to retrieve battery level");
    EspConfig.setNvsStringValue("batteryLevel", ""); // Bei Absturz während auslesen stehen keine falschen Werte im NVS
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_version_battery);
    if (pRemoteCharacteristic == nullptr)
    {
        Logger.error("MiFlora;setMiFloraSensorValues()", "Failed to find battery level characteristic UUID");
        return false;
    }
    else // read battery level
    {
        // Read the value of the characteristic...
        value = pRemoteCharacteristic->readValue();
        const char *val2 = value.c_str();
        // printf("!!!! Hex: ");
        // for (int i = 0; i < 16; i++)
        // {
        //     printf("%02x", val2[i]);
        //     printf(" ");
        // }
        // printf("\n");
        int batteryLevel = val2[0];
        // sprintf(loggerMessage, "batterylevel: %i", batteryLevel);
        // Logger.info("MiFlora;setMiFloraSensorValues()", loggerMessage);
        // float batteryLevel = (float)val2[0];
        // miFlora->batteryLevelSensor->setMeasurement(batteryLevel);
        // appendTopicAndPayload(miFlora->batteryLevelSensor, batteryLevel);

        setNvsStringIfValid(batteryLevel, "batteryLevel", 0.0, 100.0);
    }
    Logger.info("MiFlora;setMiFloraSensorValues()", "END of function");
    return true;
}

void MiFloraClass::closeBleConnection()
{
    if (_bleClient->isConnected())
        _bleClient->disconnect();
}

/**
 * Liste nach rssi sortieren ==> Comparer übergeben
 */
bool compareByRssi(const miflora_t *first, const miflora_t *second)
{
    return (first->rssiValue > second->rssiValue);
}

/**
 * Der ESP scannt die Umgebung nach erreichbaren (rssi über -90) Mifloras ab.
 * Je Neustart wird über einen zyklischen Index der nächste Miflora abgefragr und
 * dessen Werte im NVS gespeichert.
 * Da sich BLE und https offensichtlich nicht vertragen, wird der ESP neu ohne BLE gestartet
 * und die Messwerte werden aus dem NVS ausgelesen und per https an den Server übertragen.
 */
void scanMiFlorasTask(void *voidPtr)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    Logger.info("MiFlora;scanMiFlorasTask()", "start");
    MiFloraList *miFloras = (MiFloraList *)voidPtr;
    // Serial.printf("!!! miFloras in search, Count: %d\n", miFloras->size());
    BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setActiveScan(true);            //active scan uses more power, but get results faster
    pBLEScan->setInterval(0x50);
    pBLEScan->setWindow(0x30);
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Start BLE scan for %d seconds...", SCAN_TIME);
    Logger.info("MiFlora;scanMiFlorasTask()", loggerMessage);
    BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
    int count = foundDevices.getCount();
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Found BLE-devices: %d", count);
    Logger.info("MiFlora;scanMiFlorasTask()", loggerMessage);

    for (int idx = 0; idx < count; idx++)
    {
        BLEAdvertisedDevice device = foundDevices.getDevice(idx);
        if (device.haveServiceUUID())
        {
            BLEUUID serviceUUID = device.getServiceUUID();
            snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "device-uuid %s: ", serviceUUID.toString().c_str());
            if (serviceUUID.equals(miFloraUUID)) // Ist der BLE-Server ein MiFlora?
            {
                // sprintf(loggerMessage,"!!! device %d is a miflora", idx);
                // Logger.info("MiFlora;scanMiFlorasTask()", loggerMessage);
                char macAddress[LENGTH_SHORT_TEXT];
                // //esp_bd_addr_t* adrP = device.getAddress().getNative();
                // //sprintf(macAddress, "%x:%x:%x:%x:%x:%x", *adrP[0], *adrP[1], *adrP[2], *adrP[3], *adrP[4], *adrP[5]);
                strcpy(macAddress, device.getAddress().toString().c_str());
                int rssi = device.getRSSI();
                snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "MiFlora %d, address: %s, rssi: %d", idx, macAddress, rssi);
                Logger.info("MiFlora;scanMiFlorasTask()", loggerMessage);
                if (rssi >= -90) // unter -90db ist der Sensor praktisch nicht erreichbar
                {
                    miflora_t *miFlora;
                    miFlora = new miflora_t();
                    miFlora->rssiValue = rssi;
                    strcpy(miFlora->macAddress, macAddress);
                    miFloras->push_back(miFlora);
                }
            }
        }
    }
    // Serial.println("!!! Scan done!");
    miFloras->sort(compareByRssi);
    int miFlorasCounter = miFloras->size();
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Scanned mifloras: %d", miFlorasCounter);
    Logger.info("MiFlora;scanMiFlorasTask()", loggerMessage);
    if (miFloras->size() == 0)
    {
        MiFlora.closeBleConnection();
        vTaskDelete(NULL);
        return;
    }

    for (std::list<miflora_t *>::iterator it = miFloras->begin(); it != miFloras->end(); ++it)
    {
        miflora_t *mifloraPtr = *it;
        sprintf(loggerMessage, "Sorted by rssi, rssi: %.2f, mac: %s", mifloraPtr->rssiValue, mifloraPtr->macAddress);
        Logger.info("MiFlora;scanMiFlorasTask()", loggerMessage);
    }
    int miFloraIndex = EspConfig.getNvsIntValue("mifloraindex");
    if (miFloraIndex >= miFlorasCounter)
    {
        miFloraIndex = 0;
        EspConfig.setNvsIntValue("mifloraindex", 0);
    }

    // Zu lessenden MiFlora entsprechend dem im NVS gespeicherten Index suchen
    MiFloraList::iterator it = miFloras->begin();
    int index = 0;
    while (index < miFloraIndex && it != miFloras->end())
    {
        it++;
        index++;
    }
    if (it == miFloras->end())
    {
        MiFlora.closeBleConnection();
        vTaskDelete(NULL);
        return;
    }
    miflora_t *miFlora = *it;
    miFloraIndex++; // Index des nächsten MiFloras in NVS speichern
    EspConfig.setNvsIntValue("mifloraindex", miFloraIndex);
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "MiFlora %i to read: %s", miFloraIndex, miFlora->macAddress);
    Logger.info("MiFlora;scanMiFlorasTask()", loggerMessage);
    MiFlora.storeMiFloraSensorValuesToNvs(miFlora);
    MiFlora.closeBleConnection();
    SystemService.restart();
    vTaskDelete(NULL);
}

/**
 * Die erreichbaren MiFloras werden im ersten Schritt gescannt (mal sehen, welche MiFloras es gibt)
 * In den weiteren Aufrufen wird je Start nach deepsleep ein MiFlora nach dem anderen abgefragt.
 */
void MiFloraClass::readNextMiFlora()
{
    Logger.info("MiFlora;readNextMiFlora()", "start");
    // printf("!!! delete old tasks\n");
    if (_scanTask != nullptr)
        vTaskDelete(_scanTask);
    xTaskCreate(scanMiFlorasTask,   /* Task function. */
                "TaskScanMiFloras", /* String with name of task. */
                4096,               /* Stack size in words. */
                _miFloras,          /* Parameter passed as input of the task */
                1,                  /* Priority of the task. */
                _scanTask);         /* Task handle. */
}

void MiFloraClass::init()
{
    _miFloras = new MiFloraList();
    BLEDevice::init("");
    _bleClient = BLEDevice::createClient();
}

MiFloraClass MiFlora;
