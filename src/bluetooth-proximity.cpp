/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/jaredwolff/Circuit_Dojo/bluetooth-proximity/src/bluetooth-proximity.ino"
// /*
//  * Project bluetooth-proximity
//  * Description:
//  * Author:
//  * Date:
//  */

// // setup() runs once, when the device is first turned on.
// void setup() {
//   // Put initialization like pinMode and begin functions here.

// }

// // loop() runs over and over again, as quickly as it can execute.
// void loop() {
//   // The core of your code will likely live here.

// }

#include "Particle.h"
#include "helper.h"
#include "config.h"

// For debugging only
// SYSTEM_MODE(MANUAL);

// For logging
void getScanParams();
void scanResultCallback(const BleScanResult *scanResult, void *context);
void setup();
void loop();
#line 28 "/Users/jaredwolff/Circuit_Dojo/bluetooth-proximity/src/bluetooth-proximity.ino"
SerialLogHandler logHandler(115200, LOG_LEVEL_NONE, {
    { "app", LOG_LEVEL_ALL }, // enable all app messages
    // { "system", LOG_LEVEL_INFO } // only info level for system messages
});

// Local functoin to print out scan parametrs
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

    // If it has a human readable name print that too
    String name = scanResult->advertisingData.deviceName();

    // Collect the uuids showing in the advertising data
    BleUuid uuids[4];
    int uuidsAvail = scanResult->advertisingData.serviceUUID(uuids,sizeof(uuids)/sizeof(BleUuid));

    // If there are uuids, let's see if they have what we need
    if( uuidsAvail ) {
        // Print out mac info
        printBLEAddr(scanResult->address);
        Log.info("RSSI: %dBm", scanResult->rssi);

        // Loop over all available UUIDs
        // For tile devices there should only be one
        for(int i = 0; i < uuidsAvail; i++){

            // Print out the UUID we're looking for
            if( uuids[i].shorted() == TILE_UUID ) {
                Log.info("UUID: %x", uuids[i].shorted());

                // Connect to the device.
                // found = true;
                // foundAddress = scanResult->address;
                BLE.stopScanning();
                return;
            }
        }
    }

}

void setup() {
    (void)logHandler; // Does nothing, just to eliminate warning for unused variable

    // 10 second scan timeout.
    BLE.setScanTimeout(1000);
}

void loop() {
    // Scan for devices
    BLE.scan(scanResultCallback, NULL);

    // if( !found ) {
    // } else if (found && !connected) {
    //     Log.info("connecting.. %02X:%02X:%02X:%02X:%02X:%02X",
    //         foundAddress[0], foundAddress[1], foundAddress[2],
    //         foundAddress[3], foundAddress[4], foundAddress[5]);

    //     BlePeerDevice peer = BLE.connect(foundAddress,24,0,1100);

    //     if( peer.connected() ) {
    //         Log.info("connected!");
    //         connected = true;
    //     }
    //     delay(5000);
    // }

}