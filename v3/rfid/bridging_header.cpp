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

/**
 * Assuming that the sum of to tags aren't equal,
 * this functions returns the SUM of the UID entries.
 * Another approach would be, that the invoker passes a preallocated empty array
 * C could only return a pointer to an array -> this func would have to allocate heap all the time
 */
int rfid_wrapper_getUid_Sum() {
    int ret_sum = 0;
    
    if (!mfrc522->PICC_ReadCardSerial()) {
        return 0;
    }
    
    for (int i = 0; i < mfrc522.uid.size; i++) {  //
        ret_sum += mfrc522.uid.uidByte[i];
    }
    
    mfrc522.PICC_HaltA(); // Stop reading
    
}

int rfid_wrapper_hasDetected() {
    bool hasDetected = (bool) mfrc->PICC_IsNewCardPresent();
    if (hasDetected) {
        int tmp = rfid_wrapper_getUid_Sum();
        return tmp? tmp : -1;
    } else {
        return 0;
    }
}
