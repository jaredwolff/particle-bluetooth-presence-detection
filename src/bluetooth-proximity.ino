/*
 * Project bluetooth-proximity
 * Description: Check the proximity of a Tile device. Send a message when the device arrives or leaves.
 * Author: Jared Wolff
 * Date: 9/2/2019
 */

typedef enum {
    PresenceUnknown,
    Here,
    NotHere
} TilePresenceType;

const char * messages[] {
    "unknown",
    "here",
    "not here"
};

#include "Particle.h"
#include "config.h"

// For debugging only
// SYSTEM_MODE(MANUAL);

// Enable thread
// SYSTEM_THREAD(ENABLED);

// Default status
TilePresenceType present = PresenceUnknown;

// Used to store the address of the device we're looking for
BleAddress searchAddress;

// Stores the most recent data related to the device we're looking for
int8_t lastRSSI;
system_tick_t lastSeen = 0;

// The payload going to the cloud
String status;

// For logging
SerialLogHandler logHandler(115200, LOG_LEVEL_ERROR, {
    { "app", LOG_LEVEL_TRACE }, // enable all app messages
});

// Check if "Learning mode" is on
bool isLearningModeOn() {
    return (digitalRead(D7) == HIGH);
}

// Set "Learning mode" on
void setLearningModeOn() {
    digitalWrite(D7,HIGH);
}

// Set "Learning mode" off
void setLearningModeOff() {
    digitalWrite(D7,LOW);
}

// Local function to print out scan parametrs
void getScanParams() {
  BleScanParams scanParams;
  scanParams.version = BLE_API_VERSION;
  scanParams.size = sizeof(BleScanParams);
  BLE.getScanParameters(&scanParams);

  Log.info("\n interval %d\n window %d\n timeout %d\n active %d\n filter policy %d\n",
            scanParams.interval,scanParams.window,scanParams.timeout,
            scanParams.active,scanParams.filter_policy);
}

// Callback when a new device is found advertising
void scanResultCallback(const BleScanResult *scanResult, void *context) {

    // If device address doesn't match or we're not in "learning mode"
    if( !(searchAddress == scanResult->address) && !isLearningModeOn() ) {
        return;
    }

    // Collect the uuids showing in the advertising data
    BleUuid uuids[4];
    int uuidsAvail = scanResult->advertisingData.serviceUUID(uuids,sizeof(uuids)/sizeof(BleUuid));

    // Print out mac info
    BleAddress addr = scanResult->address;
    Log.trace("MAC: %02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    Log.trace("RSSI: %dBm", scanResult->rssi);

    // Loop over all available UUIDs
    // For tile devices there should only be one
    for(int i = 0; i < uuidsAvail; i++){

        // Print out the UUID we're looking for
        if( uuids[i].shorted() == TILE_UUID ) {
            Log.trace("UUID: %x", uuids[i].shorted());

            // If we're in learning mode. Save to EEprom
            if( isLearningModeOn() ) {
                searchAddress = scanResult->address;
                EEPROM.put(TILE_EEPROM_ADDRESS, searchAddress);
                setLearningModeOff();
            }

            // Save info
            lastSeen = millis();
            lastRSSI = scanResult->rssi;

            // Stop scanning
            BLE.stopScanning();

            return;
        }
    }

}

void systemEventHandler(system_event_t event, int duration, void* )
{

    // If we have a button click. Turn on the Blue led
    // that way we're in "learning mode"
    if( event == button_click ) {
        if( isLearningModeOn() ) {
            setLearningModeOff();
        } else {
            setLearningModeOn();
        }
    }

}

bool checkTileStateChanged( TilePresenceType *presence ) {

    // Check to see if it's here.
    if( millis() > lastSeen+TILE_NOT_HERE_MS ) {
        if( *presence != NotHere ) {
            *presence = NotHere;
            Log.trace("not here!");
            return true;
        }
    } else if ( lastSeen == 0 ) {
        if( *presence != PresenceUnknown ) {
            *presence = PresenceUnknown;
            Log.trace("unknown!");
            return true;
        }
    } else {
        if( *presence != Here ) {
            *presence = Here;
            Log.trace("here!");
            return true;
        }
    }

    return false;
}

void setup() {
    (void)logHandler; // Does nothing, just to eliminate warning for unused variable

    // Set LED pin
    pinMode(D7,OUTPUT);

    // Set up button event handler
    System.on(button_click, systemEventHandler);

    // Set timeout for BLE to 500ms
    BLE.setScanTimeout(50);

    // Get the search address
    EEPROM.get(TILE_EEPROM_ADDRESS, searchAddress);

    // Warning about address
    if( searchAddress == BleAddress("ff:ff:ff:ff:ff:ff") ) {
        Log.warn("Place this board into learning mode");
        Log.warn("and keep your Tile near by.");
    }
}

void loop() {

    // Reset timer
    if( lastSeen > millis() ) {
        lastSeen = 0;
    }

    // Scan for devices
    if( (millis() > lastSeen + TILE_RE_CHECK_MS) ){
        Log.trace("scan start.");
        BLE.scan(scanResultCallback, NULL);
    }

    // If we have a change
    if( checkTileStateChanged(&present) ) {

        // Get the address string
        char address[18];
        searchAddress.toString().toCharArray(address,sizeof(address));

        // Create payload
        status = String::format("{\"address\":\"%s\",\"lastSeen\":%d,\"lastRSSI\":%i,\"status\":\"%s\"}",
            address, lastSeen, lastRSSI, messages[present]);

        // Publish the RSSI and Device Info
        Particle.publish("status", status, PRIVATE, WITH_ACK);

        // Process the publish event immediately
        Particle.process();

    }

}