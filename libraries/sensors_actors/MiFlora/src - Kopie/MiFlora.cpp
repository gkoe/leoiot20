#include <MiFlora.h>
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <config.h>

#include <Thing.h>
#include <ThingConfig.h>
#include <ThingTime.h>
#include <MqttClient.h>
#include <Logger.h>

#include <sstream>

#define SCAN_TIME 30 // seconds

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

const char *miFloraUUID = "0000fe95-0000-1000-8000-00805f9b34fb";
// The characteristic of the remote service we are interested in.
static BLEUUID uuid_version_battery("00001a02-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");

static BLERemoteCharacteristic* pRemoteCharacteristic;


void MiFloraClass::publishSensorValue(const char *macAddress, const char *sensorType, float value)
{
    char payload[LENGTH_TOPIC];
    if (atoi(ThingConfig.getValue("isjsonencoded")))
    {
        _jsonBuffer.clear();
        JsonObject &jsonObject = _jsonBuffer.createObject();
        jsonObject["timestamp"] = ThingTime.getDateTime();
        jsonObject["value"] = value;
        jsonObject.printTo(payload);
    }
    else
    {
        sprintf(payload, "%.2f", value);
    }
    char fullTopic[LENGTH_TOPIC];
    sprintf(fullTopic, "%s/%s/%s", Thing.getName(), macAddress, sensorType);
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
    sprintf(loggerMessage, "Topic: %s, Payload: %s", fullTopic, payload);
    Logger.debug("Sensor set Measurement", loggerMessage);
    MqttClient.publish(fullTopic, payload);
}

bool MiFloraClass::publishMiFloraSensorValues(miflora_t *miFlora, BLEAddress bleAddress)
{
    Serial.print("!!! Forming a connection to Flora device at ");
    Serial.println(miFlora->macAddress);
    // Connect to the remove BLE Server.
    //BLEAddress* bleAddress = new BLEAddress(std::string(miFlora->macAddress));
    if (!_bleClient->connect(bleAddress))
    {
        Serial.print("!!! No connection to: ");
        Serial.println(miFlora->macAddress);
        return false;
    }
    Serial.print("Connected with: ");
    Serial.println(miFlora->macAddress);

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = _bleClient->getService(serviceUUID);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        _bleClient->disconnect();
        return false;
    }
    Serial.println(" - Found our service");

    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
    uint8_t buf[2] = {0xA0, 0x1F};
    pRemoteCharacteristic->writeValue(buf, 2, true);

    delay(500);

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
    Serial.println(pRemoteService->toString().c_str());
    if (pRemoteCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(uuid_sensor_data.toString().c_str());
        _bleClient->disconnect();
        return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    const char *val = value.c_str();

    Serial.print("Hex: ");
    for (int i = 0; i < 16; i++)
    {
        Serial.print((int)val[i], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");

    float temperature = (val[0] + val[1] * 256) / ((float)10.0);
    float moisture = (float)val[7];
    int light = val[3] + (float) (val[4] * 256);
    int conductivity = (float) (val[8] + val[9] * 256);

    publishSensorValue(miFlora->macAddress, "temperature", temperature);
    publishSensorValue(miFlora->macAddress, "moisture", moisture);
    publishSensorValue(miFlora->macAddress, "light", light);
    publishSensorValue(miFlora->macAddress, "conductivity", conductivity);

    Serial.println("Trying to retrieve battery level...");
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_version_battery);
    if (pRemoteCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(uuid_sensor_data.toString().c_str());
        _bleClient->disconnect();
        return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic...
    value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    const char *val2 = value.c_str();
    Serial.print("Hex: ");
    for (int i = 0; i < 16; i++)
    {
        Serial.print((int)val2[i], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");

    float batteryLevel = (float)val2[0];
    publishSensorValue(miFlora->macAddress, "batterylevel", batteryLevel);

    _bleClient->disconnect();
    return true;
}

void searchMiFloras(void *voidPtr)
{
    MiFloraMap *miFloras = (MiFloraMap *)voidPtr;
    // Serial.printf("!!! miFloras in search, Count: %d\n", miFloras->size());

    while (true)
    {
        BLEDevice::init("");
        BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
        // pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
        pBLEScan->setInterval(0x50);
        pBLEScan->setWindow(0x30);
        Serial.printf("\n!!! Start BLE scan for %d seconds...\n", SCAN_TIME);
        BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
        int count = foundDevices.getCount();
        int newMiFlorasCounter = 0;
        for (int i = 0; i < count; i++)
        {
            BLEAdvertisedDevice device = foundDevices.getDevice(i);
            const char *serviceUUID = device.getServiceUUID().toString().c_str();
            Serial.print("!!! ServiceUUID: ");
            Serial.println(serviceUUID);
            if (strcmp(miFloraUUID, serviceUUID) == 0) // Ist der BLE-Server ein MiFlora?
            {
                char macAddress[20];
                strcpy(macAddress, device.getAddress().toString().c_str());
                Serial.print("!!! Address: ");
                Serial.println(macAddress);
                miflora_t *miFlora;
                if (miFloras->find(macAddress) == miFloras->end())
                { // MiFlora ist nicht in Map
                    Serial.println("!!! MiFlora not in map");
                    miFlora = new miflora_t();
                    strcpy(miFlora->macAddress, macAddress);
                    miFloras->insert({miFlora->macAddress, miFlora});
                    newMiFlorasCounter++;
                    Serial.printf("New MiFlora, MAC: %s\n", miFlora->macAddress);
                    bool macFound = miFloras->find(miFlora->macAddress) != miFloras->end();
                    Serial.printf("Found MAC: %s\n", btoa(macFound));
                }
                else
                { // MiFlora ist in Map
                    Serial.println("!!! MiFlora in map");
                    miFlora = miFloras->at(macAddress);
                }
                int rssi = device.getRSSI();
                Serial.print("!!! RSSI: ");
                Serial.println(rssi);
                miFlora->rssi =rssi;
                MiFlora.publishMiFloraSensorValues(miFlora, device.getAddress());
            }
        }
        Serial.println("!!! Scan done!");
        int miFlorasCounter = miFloras->size();
        Serial.printf("!!! Mifloras: %d, new: %d\n", miFlorasCounter, newMiFlorasCounter);
        vTaskDelay(20000);
    }
}
void MiFloraClass::init()
{
    _miFloras = new MiFloraMap();
    _bleClient = BLEDevice::createClient();
    Serial.printf("!!! miFloras Count: %d\n", _miFloras->size());
    xTaskCreate(searchMiFloras,      /* Task function. */
                "TaskReconnectMqtt", /* String with name of task. */
                10000,               /* Stack size in words. */
                _miFloras,           /* Parameter passed as input of the task */
                1,                   /* Priority of the task. */
                NULL);               /* Task handle. */
}

MiFloraClass MiFlora;
