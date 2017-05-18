
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
    init_RFID_library();
}

void *detectRFID(void *arg) {
    thread_args* t_args = (thread_args*) arg;
	int cardPresent = 0;    

    while (true) {

		if(!pthread_rwlock_wrlock(t_args->lock)){
			perror("rfid_wrlock failed");
		}

        cardPresent = hasDetected_RFID();
		t_args->timestamp = get_time_us();
		t_args->data = &cardPresent;

		if(!pthread_rwlock_unlock(t_args->lock)){
			perror("rfid_unlock failed");
		}

		sleep(1); //TODO: Reasonable or no sleeptime
    }
}
