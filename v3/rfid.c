
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "helper.h"
#include "rfid.h"
#include "common.h"
#include "bridging_header.h"


void rfidSetup() {
    if (GPIO_ENABLED) {
        rfid_wrapper_setup();
    }
}

void rfidSetdown() {
    if (GPIO_ENABLED) {
        rfid_wrapper_setup();
    }
}

void *detectRFID(void *arg) {
    thread_args* t_args = (thread_args*) arg;
    struct timespec sleeptime_rfid = {0};
	int cardPresent = 0;    
    
    clock_gettime( CLOCK_MONOTONIC, sleeptime_rfid );
    while (true) {

		if(pthread_rwlock_wrlock(t_args->lock)){
			perror("rfid_wrlock failed");
		}

        if (GPIO_ENABLED) {
            cardPresent = rfid_wrapper_hasDetected();
        }
		t_args->timestamp = get_time_us();
		t_args->data = &cardPresent;

		if(pthread_rwlock_unlock(t_args->lock)){
			perror("rfid_unlock failed");
		}

        increaseTimespec(INTERVAL_INPUT * NANOSECONDS_PER_MILLISECOND, &sleeptime_us);
        sleepAbsolute(&sleeptime_rfid);
    }
}
