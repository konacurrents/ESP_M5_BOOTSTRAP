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
#ifdef M5UNIFIED

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

#else
#pragma mark START_OF_OLD_CODE

//#include "BLEServerNetworking.h"
//#include <NimBLEDevice.h>
//#include "../../Defines.h"
#define pettutorApproach

//! our server and characteristic
static NimBLEServer* _pBLEServer;
NimBLECharacteristic* _pCharacteristic;


//!device name
//unset
char *_deviceName_BLEServer = NULL;
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
    
    void onConnect(NimBLEServer* pServer)
    {
        SerialInfo.println("Client connected");
        SerialInfo.println("Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();
    };
    /** Alternative onConnect() method to extract details of the connection.
     See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc)
    {
        SerialInfo.print("Client address: ");
        SerialInfo.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         Args: connection handle, min connection interval, max connection interval
         latency, supervision timeout.
         Units; Min/Max Intervals: 1.25 millisecond increments.
         Latency: number of intervals allowed to skip.
         Timeout: 10 millisecond increments, try for 5x interval time for best results.
         */
        //GOOD:
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
        
        //try:
        //pServer->updateConnParams(desc->conn_handle, 100, 200, 10, 160);
        
    };
    void onDisconnect(NimBLEServer* pServer)
    {
        SerialInfo.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };
   
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc)
    {
        SerialInfo.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };

    /********************* Security handled here **********************
     ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        SerialDebug.println("Server Passkey Request");
        /** This should return a random 6 digit number for security
         or make your own static passkey as done here.
         */
        return 123456;
    };
    
    bool onConfirmPIN(uint32_t pass_key)
    {
        SerialDebug.print("The passkey YES/NO number: "); SerialDebug.println(pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };
   
    void onAuthenticationComplete(ble_gap_conn_desc* desc)
    {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!desc->sec_state.encrypted)
        {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            SerialDebug.println("Encrypt connection failed - disconnecting client");
            return;
        }
        SerialDebug.println("Starting BLE work!");
    };
} ;


/** Handler class for characteristic actions */
class BLEServerNetworkingCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic)  {
        Serial.printf("%s : onRead(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
    }
    
    void onWrite(NimBLECharacteristic* pCharacteristic)  {
        Serial.printf("%s : onWrite(), value: %s\n",
                      pCharacteristic->getUUID().toString().c_str(),
                      pCharacteristic->getValue().c_str());
        //TODO .. process the message
        
        //! test for sending a FEED message
        sendCommand_main((char*)pCharacteristic->getValue().c_str());
    }
};
   
/** Called before notification or indication is sent,
 the value can be changed here before sending if desired.
 */
void onNotify(NimBLECharacteristic * pCharacteristic)
{
    SerialDebug.println("Sending notification to clients");
};

#ifdef TRY_WITHOUT  //cant find "Status" ?? bizzar .. same code as prod
/** The status returned in status is defined in NimBLECharacteristic.h.
 The value returned in code is the NimBLE host return code.
 */
void onStatus(NimBLECharacteristic * pCharacteristic, Status status, int code)
{
#if (SERIAL_DEBUG_DEBUG)
    String str = ("Notification/Indication status code: ");
    str += status;
    str += ", return code: ";
    str += code;
    str += ", ";
    str += NimBLEUtils::returnCodeToString(code);
    SerialDebug.println(str);
#endif //serial_debug_debug
};
#endif

void onSubscribe(NimBLECharacteristic * pCharacteristic, ble_gap_conn_desc * desc, uint16_t subValue)
{
#if (SERIAL_DEBUG_DEBUG)
    
    String str = "Client ID: ";
    str += desc->conn_handle;
    str += " Address: ";
    str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
    if (subValue == 0) {
        str += " Unsubscribed to ";
    }
    else if (subValue == 1) {
        str += " Subscribed to notfications for ";
    }
    else if (subValue == 2) {
        str += " Subscribed to indications for ";
    }
    else if (subValue == 3) {
        str += " Subscribed to notifications and indications for ";
    }
    str += std::string(pCharacteristic->getUUID()).c_str();
    
    SerialDebug.println(str);
#endif //serial_debug_debug
};

/** Handler class for descriptor actions */
class BLEServerNetworkingDescriptorCallbacks : public NimBLEDescriptorCallbacks
{
    
    void onWrite(NimBLEDescriptor* pDescriptor)
    {
#if (SERIAL_DEBUG_DEBUG)
        
#define VERSION_1_4_1
#ifdef VERSION_1_4_1
        SerialDebug.print("Descriptor written value:");
        SerialDebug.println(pDescriptor->getValue());
#else
        std::string dscVal((char*)pDescriptor->getValue(), pDescriptor->getLength());
        SerialDebug.print("Descriptor written value:");
        SerialDebug.println(dscVal.c_str());
#endif
        
#endif
    };
    
    void onRead(NimBLEDescriptor* pDescriptor)
    {
        SerialDebug.print(pDescriptor->getUUID().toString().c_str());
        SerialDebug.println("Descriptor read");
        SerialDebug.printf(" DeviceName = %s\n", _deviceName_BLEServer?_deviceName_BLEServer:"nil");
    };
    
} ;



/** Define callback instances globally to use for multiple Charateristics \ Descriptors */
static BLEServerNetworkingDescriptorCallbacks _descriptorBLEServerCallbacks;
static BLEServerNetworkingCharacteristicCallbacks _characteristicBLEServerCallbacks;


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
    /** Optional: set the transmit power, default is 3db */
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    
    /** Set the IO capabilities of the device, each option will trigger a different pairing method.
     BLE_HS_IO_DISPLAY_ONLY    - Pass
     \ng
     */
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison
    
    /** 2 different ways to set security - both calls achieve the same result.
     no bonding, no man in the middle protection, secure connections.
     
     These are the default values, only shown here for demonstration.
     */
    //NimBLEDevice::setSecurityAuth(false, false, true);
    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
    //** NOTE: DEAD is a hex string .. and BEEF is too, so they are not really strings but UUID's in HEX.
    
    _pBLEServer = NimBLEDevice::createServer();
    _pBLEServer->setCallbacks(new BLEServerCallbacks());
    /******************  DEAD service              BEEF characteristic 2904notify,R/W/W_ENC(pairing REQUIRED!)                    ********************/
    NimBLEService* pDeadService = _pBLEServer->createService("DEAD");
    //??   NimBLEService* pDeadService = pServer->createService(SERVICE_UUID);
    NimBLECharacteristic* pBeefCharacteristic = pDeadService->createCharacteristic(
                                                                                   "BEEF",
                                                                                   //?? NimBLECharacteristic * pBeefCharacteristic = pDeadService->createCharacteristic(
                                                                                   //??    CHARACTERISTIC_UUID,
                                                                                   NIMBLE_PROPERTY::READ |
                                                                                   NIMBLE_PROPERTY::WRITE |
                                                                                   /** Require a secure connection for read and write access */
                                                                                   NIMBLE_PROPERTY::READ_ENC |  // only allow reading if paired / encrypted
                                                                                   NIMBLE_PROPERTY::WRITE_ENC   // only allow writing if paired / encrypted
                                                                                   );
    //NOTE: IS THIS RIGHT? vs the other characteristic?? 5.19.22
    //assign to global: It's the Food charactestic..
#ifdef OLD_BLE_NO_ACK_APPROACH
    _pCharacteristic = pBeefCharacteristic;
#endif
    
    
    
    /*******************************MQTT*************************************/
    //#ifdef MQTT_NETWORKING
    pBeefCharacteristic->setValue(deviceName);
    
    //#else
    //    pBeefCharacteristic->setValue("Burger");
    //#endif
    /*******************************MQTT*************************************/
    
    pBeefCharacteristic->setCallbacks(&_characteristicBLEServerCallbacks);
    
    /** 2904 descriptors are a special case, when createDescriptor is called with
     0x2904 a NimBLE2904 class is created with the correct properties and sizes.
     However we must cast the returned reference to the correct type as the method
     only returns a pointer to the base NimBLEDescriptor class.
     */
    NimBLE2904* pBeef2904 = (NimBLE2904*)pBeefCharacteristic->createDescriptor("2904");
    pBeef2904->setFormat(NimBLE2904::FORMAT_UTF8);
    pBeef2904->setCallbacks(&_descriptorBLEServerCallbacks);
    
    /********  BAAD service              F00D characteristic R/W/N  C01D Descriptor                   ********************************/
#ifdef originalApproach
    NimBLEService* pBaadService = pServer->createService("BAAD");
    NimBLECharacteristic* pFoodCharacteristic = pBaadService->createCharacteristic(
                                                                                   "F00D",
                                                                                   NIMBLE_PROPERTY::READ |
                                                                                   NIMBLE_PROPERTY::WRITE |
                                                                                   NIMBLE_PROPERTY::NOTIFY
                                                                                   );
#endif
#ifdef pettutorApproach
    NimBLEService* pBaadService = _pBLEServer->createService(_SERVICE_UUID);
    NimBLECharacteristic* pFoodCharacteristic = pBaadService->createCharacteristic(
                                                                                   _CHARACTERISTIC_UUID,
                                                                                   NIMBLE_PROPERTY::READ |
                                                                                   NIMBLE_PROPERTY::WRITE // |
                                                                                   //       NIMBLE_PROPERTY::NOTIFY
                                                                                   );
#endif
    
#ifdef originalApproach
    pFoodCharacteristic->setValue("Fries");
#endif
    
    pFoodCharacteristic->setCallbacks(&_characteristicBLEServerCallbacks);
    pFoodCharacteristic->setCallbacks(&_characteristicBLEServerCallbacks);
    
    /** Note a 0x2902 descriptor MUST NOT be created as NimBLE will create one automatically
     if notification or indication properties are assigned to a characteristic.
     */
    
#ifdef originalApproach
    /** Custom descriptor: Arguments are UUID, Properties, max length in bytes of the value */
    NimBLEDescriptor* pC01Ddsc = pFoodCharacteristic->createDescriptor(
                                                                       "C01D",
                                                                       NIMBLE_PROPERTY::READ |
                                                                       NIMBLE_PROPERTY::WRITE |
                                                                       NIMBLE_PROPERTY::WRITE_ENC, // only allow writing if paired / encrypted
                                                                       20
                                                                       );
    pC01Ddsc->setValue("Send it back!");
    pC01Ddsc->setCallbacks(&dscCallbacks);
#endif
#ifdef pettutorApproach
    /** Custom descriptor: Arguments are UUID, Properties, max length in bytes of the value */
    NimBLEDescriptor* pPetTutordsc = pFoodCharacteristic->createDescriptor(
                                                                           "C01D", //the UUID is 0xC01D
                                                                           NIMBLE_PROPERTY::READ |
                                                                           NIMBLE_PROPERTY::WRITE |
                                                                           20
                                                                           );
    pPetTutordsc->setValue("feed s/a/j type u/m ");
    pPetTutordsc->setCallbacks(&_descriptorBLEServerCallbacks);
#endif
    
#ifdef CURRENT_BLE_ACK_APPROACH
    //THIS IS RIGHT characteristic
    _pCharacteristic = pFoodCharacteristic;
    // end setup of services and characteristics
#endif
    
    //#define TRY_THIRD_SERVICE_WITH_NAME
#ifdef TRY_THIRD_SERVICE_WITH_NAME
#define DEVICE_NAME_UUID      "53636F74-7479426F79" //ScottyBoy
#define DEVICE_NAME_SERVICE      "6E616D65" //name
    /******************  DEAD service              BEEF characteristic 2904notify,R/W/W_ENC(pairing REQUIRED!)                    ********************/
    NimBLEService* pNameService = _pBLEServer->createService(DEVICE_NAME_SERVICE);
    //??   NimBLEService* pDeadService = pServer->createService(SERVICE_UUID);
    NimBLECharacteristic* pNameCharacteristic = pNameService->createCharacteristic(
                                                                                   DEVICE_NAME_UUID,
                                                                                   //?? NimBLECharacteristic * pBeefCharacteristic = pDeadService->createCharacteristic(
                                                                                   //??    CHARACTERISTIC_UUID,
                                                                                   NIMBLE_PROPERTY::READ |
                                                                                   NIMBLE_PROPERTY::WRITE |
                                                                                   /** Require a secure connection for read and write access */
                                                                                   NIMBLE_PROPERTY::READ_ENC |  // only allow reading if paired / encrypted
                                                                                   NIMBLE_PROPERTY::WRITE_ENC   // only allow writing if paired / encrypted
                                                                                   );
    
    
    /*******************************MQTT*************************************/
    //#ifdef MQTT_NETWORKING
    pNameCharacteristic->setValue(deviceName);
    
    //#else
    //    pBeefCharacteristic->setValue("Burger");
    //#endif
    /*******************************MQTT*************************************/
    
    pNameCharacteristic->setCallbacks(&_characteristicBLEServerCallbacks);
    
    /** 2904 descriptors are a special case, when createDescriptor is called with
     0x2904 a NimBLE2904 class is created with the correct properties and sizes.
     However we must cast the returned reference to the correct type as the method
     only returns a pointer to the base NimBLEDescriptor class.
     */
    NimBLE2904* pName2904 = (NimBLE2904*)pNameCharacteristic->createDescriptor("2904");
    pName2904->setFormat(NimBLE2904::FORMAT_UTF8);
    pName2904->setCallbacks(&_descriptorBLEServerCallbacks);
    pNameService->start();
    
#endif
    
    /** Start the services when finished creating all Characteristics and Descriptors */
    pDeadService->start();
    pBaadService->start();
    
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pDeadService->getUUID());
    pAdvertising->addServiceUUID(pBaadService->getUUID());
    /** If your device is battery powered you may consider setting scan response
     to false as it will extend battery life at the expense of less data sent.
     */
#ifdef TRY_THIRD_SERVICE_WITH_NAME
    pAdvertising->addServiceUUID(pNameService->getUUID());
    
#endif
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    //try the user supplied local name:  Local Name == ScottyBoy in Advertisment
    //    char *storedDeviceName = deviceName_mainModule();
    //    SerialTemp.print("Stored DeviceName = ");
    //    SerialTemp.println(storedDeviceName);
    //!setName shows up ad LocalName in Punchthrough Scanner, but
    // pAdvertising->setName(storedDeviceName);
    
    
    SerialInfo.println("Advertising Started");

}



#endif


