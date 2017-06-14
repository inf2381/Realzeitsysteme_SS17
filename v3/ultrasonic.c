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
    sched_setaffinity(0, sizeof(cpuset_sensors), &cpuset_sensors);
    thread_args* ir_args = (thread_args*) arg;

    long distance, timeDiff, timeout;
    struct timeval startTime, endTime;
    struct timespec sleeptime_us = {0};

    clock_gettime( CLOCK_MONOTONIC, &sleeptime_us );
    while (shouldRun) {
        timeout = 0;
        
        //measure distance  
        GPIO_set(PIN_TRIGGER, GPIO_HIGH);
        usleep(WAIT_TO_END_TRIGGER_ys);  // absolute sleep is not necessary here

        GPIO_set(PIN_TRIGGER, GPIO_LOW);
        
        gettimeofday(&startTime, NULL);
        endTime = startTime;
        
        if (GPIO_ENABLED) {
            while (GPIO_read(PIN_ECHO) == 0) {
                gettimeofday(&startTime, NULL);
            }
            while (GPIO_read(PIN_ECHO) == 1) {
                gettimeofday(&endTime, NULL);
                
                //timeout 
                if diff_time_us(startTime, endTime) > (200 * 1000) {
                    printf("timeout us");
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


        increaseTimespec(20 * NANOSECONDS_PER_MILLISECOND, &sleeptime_us);
        sleepAbsolute(&sleeptime_us);
        
    }
    
    pthread_exit(0);
}

