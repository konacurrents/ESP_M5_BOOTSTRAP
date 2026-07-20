//
//  BLETest.c
//  ESP_M5_BOOTSTRAP
//
//  Created by Scott Moody on 11/21/25.
//
/** Parts from: NimBLE_Client Demo:
* @see https://github.com/h2zero/NimBLE-Arduino/blob/master/examples/NimBLE_Client/NimBLE_Client.ino
*
*  Demonstrates many of the available features of the NimBLE client library.
*
*  Created: on March 24 2020
*      Author: H2zero
*/
#include "BLETest.h"


#define USE_FAST_LED
//! 7.24.25 Hot Day, Ballon last night, Mt Out
//! for the 'C' option of atom color
#ifdef USE_FAST_LED
#include "../ATOM_LED_Module/M5Display.h"
#include "../ATOM_LED_Module/LED_DisPlay.h"
#endif


#define TRY_NEW_BLE
#ifdef TRY_NEW_BLE
//#include <M5Atom.h>
#include <NimBLEDevice.h>


//!check if the string contains the other string. This is a poor man's grammer checker
bool containsSubstring2(char *message, char *substring)
{
//    if (substring.length() == 0 )
//        return false;
    bool found = strstr(&message[0], &substring[0]);
    SerialLots.printf("containsSubstring-%d - %s in: %s\n", found, &substring[0],  &message[0]);
    return found;
}

// UUIDs must match the feeder
#define SERVICE_UUID        "0000DEAD-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000BEEF-0000-1000-8000-00805f9b34fb"

static const NimBLEAdvertisedDevice* _advertisedDevice;
static bool                          _doConnect  = false;
static uint32_t                      _scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */

// BLE objects stored so can use them later...
BLEClient* _pClient;
BLERemoteCharacteristic* _pRemoteCharacteristic;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) override
    {
        SerialDebug.printf("*** Connected %s\n", pClient->getPeerAddress().toString().c_str());
        _pClient = pClient;
    }
    
    void onDisconnect(NimBLEClient* pClient, int reason) override {
        SerialDebug.printf("%s Disconnected, reason = %d - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason);
        NimBLEDevice::getScan()->start(_scanTimeMs, false, true);
    }
    
    /********************* Security handled here *********************/
    void onPassKeyEntry(NimBLEConnInfo& connInfo) override {
        SerialDebug.printf("Server Passkey Entry\n");
        /**
         * This should prompt the user to enter the passkey displayed
         * on the peer device.
         */
        NimBLEDevice::injectPassKey(connInfo, 123456);
    }
    
    void onConfirmPasskey(NimBLEConnInfo& connInfo, uint32_t passkey) override {
        SerialDebug.printf("The passkey YES/NO number: %" PRIu32 "\n", passkey);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }
    
    /** Pairing process complete, we can check the results in connInfo */
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        if (!connInfo.isEncrypted()) {
            SerialDebug.printf("Encrypt connection failed - disconnecting\n");
            /** Find the client with the connection handle provided in connInfo */
            NimBLEDevice::getClientByHandle(connInfo.getConnHandle())->disconnect();
            return;
        }
    }
} clientCallbacks;

/** Define a class to handle the callbacks when scan events are received */
class ScanCallbacks : public NimBLEScanCallbacks
{
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override
    {
        SerialLots.printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());
        //if (advertisedDevice->isAdvertisingService(NimBLEUUID("DEAD"))) {
        if (advertisedDevice->isAdvertisingService(BLEUUID(SERVICE_UUID)))
        {

            //SerialDebug.printf("Found Our Service\n");
            SerialMin.print("Device advertised: ");
            SerialMin.println(advertisedDevice->getName().c_str());
            
            //! look for ptfeeder
            //! TEMP .. use M5TestAtom ONLY
            if (containsSubstring2( (char*)advertisedDevice->getName().c_str(), (char*)"PTFeeder:M5TestAtom"))
            {
                SerialDebug.println(" ** Matches PTFeeder:");
                /** stop scan before connecting */
                NimBLEDevice::getScan()->stop();
                /** Save the device reference in a global for the client to use*/
                _advertisedDevice = advertisedDevice;
                /** Ready to connect now */
                _doConnect = true;
            }
        }
    }
    
    /** Callback to process the results of the completed scan or restart it */
    void onScanEnd(const NimBLEScanResults& results, int reason) override
    {
        SerialDebug.printf("Scan Ended, reason: %d, device count: %d; Restarting scan\n", reason, results.getCount());
        NimBLEDevice::getScan()->start(_scanTimeMs, false, true);
    }
} scanCallbacks;


/** Notification / Indication receiving handler callback */
void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)
{
    std::string str  = (isNotify == true) ? "Notification" : "Indication";
    str             += " from ";
    str             += pRemoteCharacteristic->getClient()->getPeerAddress().toString();
    str             += ": Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
    str             += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
    str             += ", Value = " + std::string((char*)pData, length);
    SerialDebug.printf("%s\n", str.c_str());
}


// Button press tracking
bool _buzzerState = false;  // false = off, true = on

void sendCommand(const char* cmd)
{
    if (_pRemoteCharacteristic && _pClient->isConnected())
    {
        _pRemoteCharacteristic->writeValue((uint8_t*)cmd, strlen(cmd), true); // write with response
        SerialDebug.print("Sent command: ");
        SerialDebug.println(cmd);
        drawpix(0, 0xFF0000); // red blink
        delay(200);
        drawpix(0, 0x0000FF); // blue idle
    } else
    {
        SerialDebug.println("Not connected to feeder");
    }
}

#ifdef OLD_V
void connectToFeeder() {
    BLEDevice::init("");
    pClient = BLEDevice::createClient();
    
    BLEScan* pScan = BLEDevice::getScan();
    pScan->setActiveScan(true);
    BLEScanResults* results = pScan->start(5);  // returns pointer in new API
    
    for (int i = 0; i < results->getCount(); i++)
    {
        BLEAdvertisedDevice device = results->getDevice(i);
        if (device.haveServiceUUID() && device.isAdvertisingService(BLEUUID(SERVICE_UUID)))
        {
            SerialDebug.println("Feeder found! Connecting...");
            if (pClient->connect(&device))
            {  // connect expects pointer
                BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
                if (pRemoteService)
                {
                    pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
                    if (pRemoteCharacteristic)
                    {
                        SerialDebug.println("Connected to feeder!");
                        drawpix(0, 0x0000FF); // blue = connected
                    }
                }
            }
            break;
        }
    }
    
    pScan->clearResults();
}
#endif

/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer()
{
    NimBLEClient* pClient = nullptr;
    
    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getCreatedClientCount())
    {
        /**
         *  Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(_advertisedDevice->getAddress());
        if (pClient)
        {
            if (!pClient->connect(_advertisedDevice, false))
            {
                SerialDebug.printf("Reconnect failed\n");
                return false;
            }
            SerialDebug.printf("Reconnected client\n");
        }
        else
        {
            /**
             *  We don't already have a client that knows this device,
             *  check for a client that is disconnected that we can use.
             */
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }
    
    /** No client to reuse? Create a new one. */
    if (!pClient)
    {
        if (NimBLEDevice::getCreatedClientCount() >= MYNEWT_VAL(BLE_MAX_CONNECTIONS))
        {
            SerialDebug.printf("Max clients reached - no more connections available\n");
            return false;
        }
        
        pClient = NimBLEDevice::createClient();
        
        SerialDebug.printf("New client created\n");
        
        pClient->setClientCallbacks(&clientCallbacks, false);
        /**
         *  Set initial connection parameters:
         *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
         *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
         *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 150 * 10ms = 1500ms timeout
         */
        pClient->setConnectionParams(12, 12, 0, 150);
        
        /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */
        pClient->setConnectTimeout(5 * 1000);
        
        if (!pClient->connect(_advertisedDevice)) {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(pClient);
            SerialDebug.printf("Failed to connect, deleted client\n");
            return false;
        }
    }
    
    if (!pClient->isConnected()) {
        if (!pClient->connect(_advertisedDevice)) {
            SerialDebug.printf("Failed to connect\n");
            return false;
        }
    }
    
    SerialDebug.printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());
    
    /** Now we can read/write/subscribe the characteristics of the services we are interested in */
    NimBLERemoteService*        pSvc = nullptr;
    NimBLERemoteCharacteristic* pChr = nullptr;
    NimBLERemoteDescriptor*     pDsc = nullptr;
    
    pSvc = pClient->getService("DEAD");
    if (pSvc) {
        pChr = pSvc->getCharacteristic("BEEF");
    }
    
    //!store so can write later ...
    _pRemoteCharacteristic = pChr;
    
    if (pChr) {
        if (pChr->canRead()) {
            SerialDebug.printf("%s Value: %s\n", pChr->getUUID().toString().c_str(), pChr->readValue().c_str());
        }
        
        if (pChr->canWrite()) {
            if (pChr->writeValue("Tasty")) {
                SerialDebug.printf("Wrote new value to: %s\n", pChr->getUUID().toString().c_str());
            } else {
                pClient->disconnect();
                return false;
            }
            
            if (pChr->canRead()) {
                SerialDebug.printf("The value of: %s is now: %s\n", pChr->getUUID().toString().c_str(), pChr->readValue().c_str());
            }
        }
        
        if (pChr->canNotify()) {
            if (!pChr->subscribe(true, notifyCB)) {
                pClient->disconnect();
                return false;
            }
        } else if (pChr->canIndicate()) {
            /** Send false as first argument to subscribe to indications instead of notifications */
            if (!pChr->subscribe(false, notifyCB)) {
                pClient->disconnect();
                return false;
            }
        }
    } else {
        SerialDebug.printf("DEAD service not found.\n");
    }
    
    pSvc = pClient->getService("BAAD");
    if (pSvc) {
        pChr = pSvc->getCharacteristic("F00D");
        if (pChr) {
            if (pChr->canRead()) {
                SerialDebug.printf("%s Value: %s\n", pChr->getUUID().toString().c_str(), pChr->readValue().c_str());
            }
            
            pDsc = pChr->getDescriptor(NimBLEUUID("C01D"));
            if (pDsc) {
                SerialDebug.printf("Descriptor: %s  Value: %s\n", pDsc->getUUID().toString().c_str(), pDsc->readValue().c_str());
            }
            
            if (pChr->canWrite()) {
                if (pChr->writeValue("No tip!")) {
                    SerialDebug.printf("Wrote new value to: %s\n", pChr->getUUID().toString().c_str());
                } else {
                    pClient->disconnect();
                    return false;
                }
                
                if (pChr->canRead()) {
                    SerialDebug.printf("The value of: %s is now: %s\n",
                                  pChr->getUUID().toString().c_str(),
                                  pChr->readValue().c_str());
                }
            }
            
            if (pChr->canNotify()) {
                if (!pChr->subscribe(true, notifyCB)) {
                    pClient->disconnect();
                    return false;
                }
            } else if (pChr->canIndicate()) {
                /** Send false as first argument to subscribe to indications instead of notifications */
                if (!pChr->subscribe(false, notifyCB)) {
                    pClient->disconnect();
                    return false;
                }
            }
        }
    } else {
        SerialDebug.printf("BAAD service not found.\n");
    }
    
    SerialDebug.printf("Done with this device!\n");
    return true;
}

//! new version for 2.x
void setupFeeder()
{
    SerialDebug.printf("Starting NimBLE Client\n");
    
    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("NimBLE-Client");
    
    /**
     * Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_KEYBOARD_ONLY   - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY); // use passkey
    // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison
    
    /**
     * 2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, BLE secure connections.
     *  These are the default values, only shown here for demonstration.
     */
    // NimBLEDevice::setSecurityAuth(false, false, true);
    // NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM | BLE_SM_PAIR_AUTHREQ_SC);
    
    /** Optional: set the transmit power */
    NimBLEDevice::setPower(3); /** 3dbm */
    NimBLEScan* pScan = NimBLEDevice::getScan();
    
    /** Set the callbacks to call when scan events occur, no duplicates */
    pScan->setScanCallbacks(&scanCallbacks, false);
    
    /** Set scan interval (how often) and window (how long) in milliseconds */
    pScan->setInterval(100);
    pScan->setWindow(100);
    
    /**
     * Active scan will gather scan response data from advertisers
     *  but will use more energy from both devices
     */
    pScan->setActiveScan(true);
    
    /** Start scanning for advertisers */
    pScan->start(_scanTimeMs);
    SerialDebug.printf("Scanning for peripherals\n");
}

//! main entry point for setup_BLETest
void setup_BLETest()
{
    //M5.begin(true, false, true);
    //Serial.begin(115200);
    drawpix(0, 0x000000); // off
    
    //connectToFeeder();
    setupFeeder();
}

#pragma mark BUTTON Processing of M5
//! 10.16.25 add buttons
boolean _shortPressA_MainModule = false;
boolean _longPressA_MainModule = false;
boolean _longLongPressA_MainModule = false;
boolean _shortPressB_MainModule = false;
boolean _longPressB_MainModule = false;
boolean _longLongPressB_MainModule = false;
//!big button on front of M5StickC Plus
void checkButtonA_MainModule();
void checkButtonB_MainModule();

void loop_BLETest()
{
    M5.update();
    
    //! check button presses
    checkButtonA_MainModule();
    checkButtonB_MainModule();
    
    //! see if connected (set above)
    if (_doConnect)
    {
        _doConnect = false;
        /** Found a device we want to connect to, do it now */
        if (connectToServer())
        {
            SerialDebug.println("!! Connected to BLE Server.");
            //callCallbacksMain ...
            
            _doConnect = false; // TRY 6.11.22 (Family Day)
            //return; /// try 4.22.22
            
            //! don't scan anymore ..
            sendCommand("s");

        }
        else
        {
            SerialDebug.printf("Failed to connect, starting scan\n");
            
            //! scan more ..
            NimBLEDevice::getScan()->start(_scanTimeMs, false, true);
        }
    }
  
    //! look for button presses (which isn't working)
    if (_shortPressA_MainModule || _shortPressB_MainModule)
    {
        SerialDebug.println("_shortPress");
        // Short press: send feed command
        sendCommand("s");
    }
    else if (_longLongPressA_MainModule || _longLongPressB_MainModule)
    {
        SerialDebug.println("_longLongPress");

        // Press and hold (>= 2 seconds): toggle buzzer
        _buzzerState = !_buzzerState;
        sendCommand(_buzzerState ? "B" : "b");
    }
    
    delay(50);
}

//!big button on front of M5StickC Plus
void checkButtonA_MainModule()
{
    _shortPressA_MainModule = false;
    _longPressA_MainModule = false;
    _longLongPressA_MainModule = false;
#ifdef ESP_M5
    
    //!NOTE: ths issue is the timer is interruped by the scanner.. so make long-long very long..
    //was 1000  (from 500)
    if (M5.BtnA.wasReleasefor(4500))
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** LONG LONG PRESS ***");
        _longLongPressA_MainModule = true;
    }
    else if (M5.BtnA.wasReleasefor(1000))
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** LONG PRESS ***");
        _longPressA_MainModule = true;
    }
    else if (M5.BtnA.wasReleased())
    {
        //        buttonA_shortPress_MainModule();
        SerialDebug.println("MainModule **** SHORT PRESS ***");
        _shortPressA_MainModule = true;
    }
#endif
}
//!big button on front of M5StickC Plus
void checkButtonB_MainModule()
{
    _shortPressB_MainModule = false;
    _longPressB_MainModule = false;
    _longLongPressB_MainModule = false;
#ifdef ESP_M5
    
    //!NOTE: ths issue is the timer is interruped by the scanner.. so make long-long very long..
    //was 1000  (from 500)
    if (M5.BtnB.wasReleasefor(4500))
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** LONG LONG PRESS ***");
        _longLongPressB_MainModule = true;
    }
    else if (M5.BtnB.wasReleasefor(1000))
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** LONG PRESS ***");
        _longPressB_MainModule = true;
    }
    else if (M5.BtnB.wasReleased())
    {
        //        buttonA_shortPress_MainModule();
        SerialDebug.println("MainModule **** SHORT PRESS ***");
        _shortPressB_MainModule = true;
    }
#endif
}
#else

void setup_BLETest() {
   
}

void loop_BLETest() {
}
#endif
