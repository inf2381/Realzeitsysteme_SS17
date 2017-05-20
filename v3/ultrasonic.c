/* https://tutorials-raspberrypi.de/entfernung-messen-mit-ultraschallsensor-hc-sr04/ */


#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#include "common.h"
#include "ultrasonic.h"
#include "gpio.h"
#include "helper.h"



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
    thread_args* ir_args = (thread_args*) arg;

    long distance, timeDiff;
    struct timeval startTime, endTime;
    struct timespec sleeptime;
    
    while (true) {
        
        //measure distance  
        GPIO_set(PIN_TRIGGER, GPIO_HIGH);
        sleepAbsolute(WAIT_TO_END_TRIGGER_ys * NANOSECONDS_PER_MICROSECOND, &sleeptime);

        GPIO_set(PIN_TRIGGER, GPIO_LOW);
        
        gettimeofday(&startTime, NULL);
        endTime = startTime;
        
        while (GPIO_read(PIN_ECHO) == 0) {
            gettimeofday(&startTime, NULL);
        }
        while (GPIO_read(PIN_ECHO) == 1) {
            gettimeofday(&endTime, NULL);
        }
        
        timeDiff = diff_time_us(startTime, endTime);
        distance = (timeDiff * SONIC_SPEED) / 2;
        
       
        
		//ipc to main
		if(pthread_rwlock_wrlock(ir_args->lock)){
			perror("us_lock failed");
		}

		ir_args->timestamp = get_time_us();
		ir_args->data = &distance;
		
		if(pthread_rwlock_unlock(ir_args->lock)){
			perror("us_lock failed");
		}


        sleepAbsolute(INTERVAL_INPUT * NANOSECONDS_PER_MILLISECOND, &sleeptime);
        
    }
}

