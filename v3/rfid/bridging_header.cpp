#include "bridging_header.h"
#include "MFRC522.h"

#include <unistd.h>

MFRC522* mfrc;

void rfid_wrapper_setup() {
    mfrc = new MFRC522();
    mfrc->PCD_Init();
}

void rfid_wrapper_setdown() {
    delete mfrc;
}

int rfid_wrapper_hasDetected() {
    bool hasDetected = (bool) mfrc->PICC_IsNewCardPresent();
    if (hasDetected) {
        return 1;
    } else {
        return 0;
    }
}
