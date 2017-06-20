#define _GNU_SOURCE
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
    sched_setaffinity(0, sizeof(cpuset_sensors), &cpuset_sensors);
    thread_setPriority(PRIO_SENSORS);
    
    thread_args* t_args = (thread_args*) arg;
    struct timespec sleeptime_rfid = {0};
	int cardPresent = 0;    
    
    clock_gettime(CLOCK_MONOTONIC, &sleeptime_rfid );


    while (shouldRun) {

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

        increaseTimespec(INTERVAL_INPUT_US * NANOSECONDS_PER_MICROSECOND, &sleeptime_rfid);
        sleepAbsolute(&sleeptime_rfid);
    }
    
    pthread_exit(0);
}
