#ifndef HELPER_H
#define HELPER_H

#include "Particle.h"

void printBLEAddr(BleAddress addr) {

  Log.info("MAC: %02X:%02X:%02X:%02X:%02X:%02X",
  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

}

#endif //HELPER_H