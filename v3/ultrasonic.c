#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>

#include "gpio.h"
#include "helper.h"
#include "ultrasonic.h"
#include "common.h"




void ultrasonicSetup() {
	GPIO_export(PIN_TRIGGER);
	GPIO_setDirection(PIN_TRIGGER, PIN_OUT);

	GPIO_export(PIN_ECHO);
	GPIO_setDirection(PIN_ECHO, PIN_IN);
}


void ultrasonicSetdown() {
	GPIO_unexport(PIN_TRIGGER);
	GPIO_unexport(PIN_ECHO);
}


void *measureDistance(void *arg) {
#ifdef TIMEMEASUREMENT  //see common.h
    struct timespec start_time = {0};
    struct timespec end_time = {0};
    long long *buffer = getTimeBuffer(BUF_SIZE);  // getBuf in helper.h; BUF_SIZE in common.h
    int current_index = 0;
#endif

    sched_setaffinity(0, sizeof(cpuset_sensors), &cpuset_sensors);
    thread_setPriority(PRIO_SENSORS);
    
    thread_args* ir_args = (thread_args*) arg;

    long distance, timeDiff, timeout;
    struct timeval startTime, endTime;
    struct timespec sleeptime_us = {0};

    clock_gettime( CLOCK_MONOTONIC, &sleeptime_us );
    while (shouldRun) {
#ifdef TIMEMEASUREMENT
        clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif
        timeout = 0;
        
        //measure distance  
        GPIO_set(PIN_TRIGGER, GPIO_HIGH);
        usleep(WAIT_TO_END_TRIGGER_US);  // absolute sleep is not necessary here

        GPIO_set(PIN_TRIGGER, GPIO_LOW);
        
        gettimeofday(&startTime, NULL);
        endTime = startTime;
        
        if (GPIO_ENABLED) {
            while (GPIO_read(PIN_ECHO) == 0) {
                gettimeofday(&startTime, NULL);
            }
            while (GPIO_read(PIN_ECHO) == 1) {
                gettimeofday(&endTime, NULL);
                
                //timeout 200ms
                if (diff_time_us(startTime, endTime) > (200 * 1000)) {
                    if (VERBOSE_DEF) {
                        printf("timeout us");
                    }    
                    timeout = 1;
                    break;
                }
            }
        }
        
        if (!timeout) {
            timeDiff = diff_time_us(startTime, endTime);
            distance = (timeDiff * SONIC_SPEED) / 2;
        } else {
            distance = INT_MAX;
        }
        
       
        
		//ipc to main
		if(pthread_rwlock_wrlock(ir_args->lock)){
			perror("us_lock failed");
		}

		ir_args->timestamp = get_time_us();
		ir_args->data = &distance;
		
		if(pthread_rwlock_unlock(ir_args->lock)){
			perror("us_lock failed");
		}


        increaseTimespec(INTERVAL_ULTRASONIC_US, &sleeptime_us);
        
#ifdef TIMEMEASUREMENT
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        appendToBuf(buffer, &current_index, diff_time_ns(&start_time, &end_time));
#endif
        
        sleepAbsolute(&sleeptime_us);
    }
    
#ifdef TIMEMEASUREMENT
    logToCSV("log_ultrasonic.csv", buffer);
#endif
    
    pthread_exit(0);
}

