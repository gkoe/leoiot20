#include <MiFloraHttps.h>

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
#include <IotSensor.h>
#include <Thing.h>
#include <SystemService.h>
#include <HttpClient.h>

#define SCAN_TIME 10 // seconds

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");
static BLEUUID miFloraUUID("0000fe95-0000-1000-8000-00805f9b34fb");

//const char *miFloraUUID = "0000fe95-0000-1000-8000-00805f9b34fb";
// The characteristic of the remote service we are interested in.
static BLEUUID uuid_version_battery("00001a02-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");

static BLERemoteCharacteristic *pRemoteCharacteristic;

const char *urlMifloras = "leonding.synology.me/esplogs/mifloraentries";
const char *basicAuthenticationName = "gerald";
const char *basicAuthenticationPassword = "piKla87Sie57";

/**
 * Messwert per https an den Server übertragen
 */
void sendByHttps(const char *mac, const char *sensorName, float value)
{
    char topic[LENGTH_TOPIC];
    char payload[LENGTH_PAYLOAD];
    char body[LENGTH_LOGGER_MESSAGE];

    // sprintf(topic, "\"miflora/%s/%s/state\"", mac, sensorName);
    // sprintf(payload, "{\"timestamp\": %ld,\"value\": %.2f}", EspTime.getTime(), value);
    // sprintf(body, "{\"topic\": %s,\"payload\": %s}", topic, payload);
    sprintf(body, "{\"mac\": \"%s\",\"moisture\": %s,\"temperature\": %s,\"brightness\": %s,\"batteryLevel\": %s}",
		      mac, "99", "33", "9999", "99");
    Logger.debug("MiFloraGateway, send by https: ", body);
    HttpClient.post(urlMifloras, body, true, "gerald", "piKla87Sie57");
}

/**
 * Messwerte des aktuellen Miflora auslesen und per https an den Server übertragen.
 */
void MiFloraHttpsClass::getAndSendMifloraValues()
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    BLEAddress bleAddress = BLEAddress(std::string(_miflora.macAddress));
    if (!_bleClient->connect(bleAddress))
    {
        snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "No connection to: %s", _miflora.macAddress);
        Logger.error("MiFlora;getAndSendMifloraValues()", loggerMessage);
        return;
    }
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Connected with: %s", _miflora.macAddress);
    Logger.info("MiFlora;getAndSendMifloraValues()", loggerMessage);

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = _bleClient->getService(serviceUUID);
    if (pRemoteService == nullptr)
    {
        Logger.error("MiFlora;getAndSendMifloraValues()", "Failed to find our service UUID");
        return;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
    uint8_t buf[2] = {0xA0, 0x1F}; // Laut Protokoll zuerst die zwei Bytes schreiben und dann die Werte lesen
    pRemoteCharacteristic->writeValue(buf, 2, true);
    Logger.info("MiFlora;getAndSendMifloraValues()", "written A0 1F to characteristics");
    vTaskDelay(1000 / portTICK_PERIOD_MS); //! war 500
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
    if (pRemoteCharacteristic == nullptr)
    {
        Logger.error("MiFlora;getAndSendMifloraValues()", "Failed to find our characteristic UUID");
        closeBleConnection();
    }
    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Found our characteristic UUID");
    Logger.info("MiFlora;getAndSendMifloraValues()", loggerMessage);

    const char *val = value.c_str();

    float temperature = (val[0] + val[1] * 256) / ((float)10.0);
    float moisture = (float)val[7];
    int brightness = val[3] + (float)(val[4] * 256);
    int conductivity = (float)(val[8] + val[9] * 256);

    sendByHttps(_miflora.macAddress, "rssi", _miflora.rssi);
    sendByHttps(_miflora.macAddress, "moisture", moisture);
    sendByHttps(_miflora.macAddress, "temperature", temperature);
    sendByHttps(_miflora.macAddress, "brightness", brightness);
    sendByHttps(_miflora.macAddress, "conductivity", conductivity);

    Logger.info("MiFlora;setMiFloraSensorValues()", "Trying to retrieve battery level");
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_version_battery);
    if (pRemoteCharacteristic == nullptr)
    {
        Logger.error("MiFlora;setMiFloraSensorValues()", "Failed to find battery level characteristic UUID");
        return;
    }
    else
    {
        // Read the value of the characteristic...
        value = pRemoteCharacteristic->readValue();
        const char *val2 = value.c_str();
        int batteryLevel = val2[0];
        sendByHttps(_miflora.macAddress, "batterylevel", batteryLevel);
    }
    return;
}

void MiFloraHttpsClass::closeBleConnection()
{
    if (_bleClient->isConnected())
        _bleClient->disconnect();
}

bool MiFloraHttpsClass::getNextMiflora()
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    Logger.info("MiFlora;getNextMiflora()", "start");
    BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
    // alle erreichbaren BLE-Devoces suchen
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(0x50);
    pBLEScan->setWindow(0x30);
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Start BLE scan for %d seconds...", SCAN_TIME);
    Logger.info("MiFlora;getNextMiflora()", loggerMessage);
    BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
    int count = foundDevices.getCount();
    if (count == 0)
    {
        Logger.error("MiFlora;getNextMiflora()", "No Ble-devices found");
        return false;
    }

    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Found BLE-devices: %d", count);
    Logger.info("MiFlora;getNextMiflora()", loggerMessage);
    int miFloraIndex = EspConfig.getNvsIntValue("mifloraindex"); // Index des zu übertragenden Mifloras
    int foundMiflorasCounter = 0;
    for (int idx = 0; idx < count; idx++)
    {
        BLEAdvertisedDevice device = foundDevices.getDevice(idx);
        if (device.haveName())
        {
            snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "getDevice() deviceName: %s", device.getName().c_str());
            Logger.info("MiFlora;getNextMiflora()", loggerMessage);
        }
        if (device.haveServiceUUID())
        {
            BLEUUID serviceUUID = device.getServiceUUID();
            snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "device-uuid %s: ", serviceUUID.toString().c_str());
            if (serviceUUID.equals(miFloraUUID)) // Ist der BLE-Server ein MiFlora?
            {
                char macAddress[LENGTH_SHORT_TEXT];
                strcpy(macAddress, device.getAddress().toString().c_str());
                int rssi = device.getRSSI();
                snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "MiFlora %d, address: %s, rssi: %d", idx, macAddress, rssi);
                Logger.info("MiFlora;getNextMiflora()", loggerMessage);
                if (rssi >= -90)
                {
                    if (foundMiflorasCounter == 0 || foundMiflorasCounter == miFloraIndex) // wenn der gesuchte Index nicht existiert bleibt 0 als Defaultindex
                    {
                        _miflora.rssi = rssi;
                        strcpy(_miflora.macAddress, macAddress);
                    }
                    foundMiflorasCounter++;
                }
            }
        }
    }
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Scanned mifloras: %d, nexMiflora: %s", foundMiflorasCounter, _miflora.macAddress);
    Logger.info("MiFlora;getNextMiflora()", loggerMessage);
    MiFloraHttps.closeBleConnection();
    if (miFloraIndex >= foundMiflorasCounter)
    {
        miFloraIndex = 0;
        EspConfig.setNvsIntValue("mifloraindex", 0);
    }
    else
    {
        EspConfig.setNvsIntValue("mifloraindex", miFloraIndex + 1);
    }
    return true;
}

void scanMiFlorasTask(void *voidPtr)
{
    if (MiFloraHttps.getNextMiflora())
    {
        MiFloraHttps.getAndSendMifloraValues();
    }
    vTaskDelete(NULL);
}

void MiFloraHttpsClass::init()
{
    BLEDevice::init("");
    _bleClient = BLEDevice::createClient();
    xTaskCreate(scanMiFlorasTask,   /* Task function. */
                "TaskScanMiFloras", /* String with name of task. */
                4096,               /* Stack size in words. */
                nullptr,            /* Parameter passed as input of the task */
                1,                  /* Priority of the task. */
                _scanTask);         /* Task handle. */
}

MiFloraHttpsClass MiFloraHttps;
