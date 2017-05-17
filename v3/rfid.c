
#include <pthread.h>
#include <stdbool.h>

#include "rfid.h"
#include "common.h"
#include "../bridging_header.h"

void rfidSetup() {
    init_RFID_library();
}

void *detectRFID(void *arg) {
    thread_args* ir_args = (thread_args*) arg;
    
    while (true) {
        hasDetected_RFID();
    }
}
