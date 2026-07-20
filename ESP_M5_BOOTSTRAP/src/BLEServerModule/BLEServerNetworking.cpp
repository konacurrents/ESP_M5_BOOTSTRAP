//
//  BLEServerNetworking.cpp
//  ESP_M5_BOOTSTRAP
//
//  Created by Scott Moody on 6/7/26.
//

/** Parts from: NimBLE_Client Demo:
 * @see https://github.com/h2zero/NimBLE-Arduino/blob/master/examples/NimBLE_Client/NimBLE_Client.ino
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 */
#include "BLEServerNetworking.h"

#include <NimBLEDevice.h>

//! our server and characteristic
static NimBLEServer* _pBLEServer;
NimBLECharacteristic* _pCharacteristic;

//!device name
//unset char *_deviceName_BLEServer = NULL;
//!full name
char _serviceName_BLEServer[100];

//! save the string version of these 2 attributes
char *_SERVICE_UUID;
char *_CHARACTERISTIC_UUID;

#define MAX_MESSAGE 600
char _asciiMessage[MAX_MESSAGE];


//! retrieve the service name (PTFEEDER, PTFeeder:Name, PTClicker:Name, etc)
char *getServiceName_BLEServerNetworking()
{
    return _serviceName_BLEServer;
}


//!send ACK over bluetooth, this right now is 0x01
void sendBLEMessageACKMessage()
{
    SerialTemp.printf("%d sendBLEMessageACKMessage._pCharacteristic\n", _pCharacteristic);
    _pCharacteristic->setValue(0x01);  //??  This is the acknowlege(ACK) back to client.  Later this should be contigent on a feed completed
}

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class BLEServeNetworkingCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client connected:\n%s", connInfo.toString().c_str());
        
        /**
         *  We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments.
         */
        pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
    }
    
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.printf("Client disconnected - start advertising\n");
        NimBLEDevice::startAdvertising();
    }
    
    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
    }

    /********************* Security handled here *********************/
    uint32_t onPassKeyDisplay() override {
        Serial.printf("Server Passkey Display\n");
        /**
         * This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    }
    
    void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t passkey) override {
        Serial.printf("The passkey YES/NO number: %" PRIu32 "\n", passkey);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }
    
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!connInfo.isEncrypted()) {
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            Serial.printf("Encrypt connection failed - disconnecting client\n");
            return;
        }
        
        Serial.printf("Secured connection to: %s\n", connInfo.getAddress().toString().c_str());
    }
} _BLEServeNetworkingCallbacks;


/** Handler class for characteristic actions */
class BLEServerNetworkingCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onRead(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }
    
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s : onWrite(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
        //TODO .. process the message
        
        //! test for sending a FEED message
        sendCommand_main((char*)pCharacteristic->getValue().c_str());
    }
    
    /**
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
        SerialLots.printf("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
    }
    
    /** Peer subscribed to notifications/indications */
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        std::string str  = "Client ID: ";
        str             += connInfo.getConnHandle();
        str             += " Address: ";
        str             += connInfo.getAddress().toString();
        if (subValue == 0) {
            str += " Unsubscribed to ";
        } else if (subValue == 1) {
            str += " Subscribed to notifications for ";
        } else if (subValue == 2) {
            str += " Subscribed to indications for ";
        } else if (subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID());
        
        Serial.printf("%s\n", str.c_str());
    }
} _BLEServerNetworkingCharacteristicCallbacks;

/** Handler class for descriptor actions */
class BLEServerNetworkingDescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
        std::string dscVal = pDescriptor->getValue();
        Serial.printf("Descriptor written value: %s\n", dscVal.c_str());
    }
    
    void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
        Serial.printf("%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
        SerialDebug.printf(" DeviceName = %s\n", _serviceName_BLEServer?_serviceName_BLEServer:"nil");

    }
} _BLEServerNetworkingDescriptorCallbacks;




//!the 'setup' for this module BLEServerNetworking. Here the service name is added (and potentially more later)
void setup_BLEServerNetworking(char *serviceName, char * deviceName, char *serviceUUID, char *characteristicUUID)
{
    
    _SERVICE_UUID = serviceUUID;
    _CHARACTERISTIC_UUID = characteristicUUID;
    char *storedDeviceName = deviceName; //todo .. call the original code
    
    SerialMin.printf("setup_BLEServerNetworking(%s,%s,%s,%s)\n", serviceName?serviceName:"NULL", deviceName?deviceName:"NULL", serviceUUID?serviceUUID:"NULL", characteristicUUID?characteristicUUID:"NULL");
    
    sprintf(_serviceName_BLEServer, "%s:%s", serviceName, storedDeviceName);
    SerialMin.printf("Setting BLE serviceName: '%s'\n", _serviceName_BLEServer);
    //!! Seems to be a length of 31 too long
    //!    //DEBUG: crashed
    // assert failed: static void NimBLEDevice::init(const string&) NimBLEDevice.cpp:901 (rc == 0)
    // void NimBLEDevice::init(const std::string &deviceName) {
    //!@see https://github.com/espressif/arduino-esp32/issues/7894
    int len = strlen(_serviceName_BLEServer);
    if (len > 29)
    {
        _serviceName_BLEServer[28] = 0;
        SerialMin.printf("Name was over 29 characters, so shirking: '%s'\n", _serviceName_BLEServer);
    }
    
#pragma mark START BLE Server
    //! 6.8.26 (but from 2022)
    //! NOTE: we added the DEAD service to our ESP32 based devices,
    //! just to distinguish between a GEN3 device - as they won't have a DEAD service
    //! We do NOT use the DEAD service itself
    NimBLEDevice::init(_serviceName_BLEServer);
    
    //!create server
    _pBLEServer = NimBLEDevice::createServer();
    
    //! set callbacks
    _pBLEServer->setCallbacks(&_BLEServeNetworkingCallbacks);
    
#pragma mark DEAD Service
    //! create the DEAD service so a GEN3 can be ruled out..
    NimBLEService* pDeadService = _pBLEServer->createService("DEAD");
    NimBLECharacteristic* pBeefCharacteristic = pDeadService->createCharacteristic("BEEF",NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    
    pBeefCharacteristic->setValue("Beef");
    pBeefCharacteristic->setCallbacks(&_BLEServerNetworkingCharacteristicCallbacks);
    
    /** 2904 descriptors are a special case, when createDescriptor is called with
     0x2904 a NimBLE2904 class is created with the correct properties and sizes.
     However we must cast the returned reference to the correct type as the method
     only returns a pointer to the base NimBLEDescriptor class.
     */
    NimBLE2904* pBeef2904 = (NimBLE2904*)pBeefCharacteristic->createDescriptor("2904");
    pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
    pBeef2904->setCallbacks(&_BLEServerNetworkingDescriptorCallbacks);
    //! used??
    
#pragma mark SERVICE_UUID service (main service)
    //! create our main _SERVICE_UUID  service`
    NimBLEService* pIDogWatchService = _pBLEServer->createService(_SERVICE_UUID);
    NimBLECharacteristic* pIDogWatchCharacteristic = pIDogWatchService->createCharacteristic(_CHARACTERISTIC_UUID,NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    
    pIDogWatchCharacteristic->setValue(_serviceName_BLEServer);
    pIDogWatchCharacteristic->setCallbacks(&_BLEServerNetworkingCharacteristicCallbacks);
    //pIDogWatchCharacteristic->setValue((const uint8_t*)deviceName, strlen(deviceName));

    
    /** Custom descriptor: Arguments are UUID, Properties, max length in bytes of the value */
    NimBLEDescriptor* pPetTutordsc = pIDogWatchCharacteristic->createDescriptor("C01D",NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC, 20);
    pPetTutordsc->setValue(_serviceName_BLEServer);
    pPetTutordsc->setCallbacks(&_BLEServerNetworkingDescriptorCallbacks);
    
    //! now the advertising..
    //!
    _pCharacteristic = pIDogWatchCharacteristic;
        
    /** Create an advertising instance and add the services to the advertised data */
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName(_serviceName_BLEServer);
    
    //! advertise service UUID for DEAD
    pAdvertising->addServiceUUID(pDeadService->getUUID());

    //! advertise service UUID for iDOGWatch
    pAdvertising->addServiceUUID(pIDogWatchService->getUUID());
    /**
     *  If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->enableScanResponse(true);
    pAdvertising->start();
    
    Serial.printf("Advertising Started\n");
    
}

//! main loop
void loop_BLEServerNetworking()
{
    if (_pBLEServer && _pBLEServer->getConnectedCount() >0 )
    {
        NimBLEService* pSvc = _pBLEServer->getServiceByUUID(_SERVICE_UUID);
        if (pSvc)
        {
            NimBLECharacteristic* pChr = pSvc->getCharacteristic(_CHARACTERISTIC_UUID);
            if (pChr)
            {
                pChr->notify();
            }
        }
    }
    
}

