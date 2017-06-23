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
#ifdef TIMEMEASUREMENT:  //see common.h
    struct timespec start_time = {0};
    struct timespec end_time = {0};
    long long *buffer = getTimeBuffer(BUF_SIZE);  // getBuf in helper.h; BUF_SIZE in common.h
#endif
    sched_setaffinity(0, sizeof(cpuset_sensors), &cpuset_sensors);
    thread_setPriority(PRIO_SENSORS);
    
    thread_args* t_args = (thread_args*) arg;
    struct timespec sleeptime_rfid = {0};
	int cardPresent = 0;    
    
    clock_gettime(CLOCK_MONOTONIC, &sleeptime_rfid );


    while (shouldRun) {
#ifdef TIMEMEASUREMENT:
        clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif
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
#ifdef TIMEMEASUREMENT:
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        appendToBuf(buffer, diff_time_ns(&start_time, &end_time));
#endif
    }
    
#ifdef TIMEMEASUREMENT
    logToCSV("log_rfid.csv", buffer);
#endif

    
    pthread_exit(0);
}
