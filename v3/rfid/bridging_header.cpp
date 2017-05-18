#include "bridging_header.h"
#include "MFRC522.h"

#include <unistd.h>

MFRC522 mfrc;

void init_RFID_library() {
    mfrc.PCD_Init();
}


int hasDetected_RFID() {
    bool hasDetected = (bool) mfrc.PICC_IsNewCardPresent();
    if (hasDetected) {
        return 1;
    } else {
        return 0;
    }
}
