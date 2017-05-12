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



void ultrasonic_Setup() {
	GPIO_export(PIN_TRIGGER);
	GPIO_setDirection(PIN_TRIGGER, PIN_OUT);

	GPIO_export(PIN_ECHO);
	GPIO_setDirection(PIN_ECHO, PIN_IN);
}


void ultrasonicSetdown() {
	GPIO_unexport(PIN_TRIGGER);
	GPIO_unexport(PIN_ECHO);
}


void *exploitDistance(void *arg) {
    int pipeFD = *((int *) arg);
    ssize_t resultRead;
    long readDistance;
    
    while (true) {
        resultRead = read(pipeFD, &readDistance, __SIZEOF_LONG__);
        if (resultRead != __SIZEOF_LONG__) {
            perror("read");
            exit(3);
        }
		printf("Distance: %ld\n", readDistance);        
        sleep(1); //TODO: Reasonable or no sleeptime
    }
}

void *measureDistance(void *arg) {
    thread_args* ir_args = (thread_args*) arg;

    long distance;
    long startTime, endTime, timeDiff;
    
    while (true) {
        
        //measure distance  
        GPIO_set(PIN_TRIGGER, GPIO_HIGH);
        
        usleep(WAIT_TO_END_TRIGGER_ys);
        GPIO_set(PIN_TRIGGER, GPIO_LOW);
        
        startTime = get_time_us();
        endTime = startTime;
        
        while (GPIO_read(PIN_ECHO) == 0) {
            startTime = get_time_us();
        }
        while (GPIO_read(PIN_ECHO) == 1) {
            endTime = get_time_us();
        }
        
        timeDiff = endTime - startTime;
		printf("Timedifference: %ld\n", timeDiff);
        distance = (timeDiff * SONIC_SPEED) / 2;
		printf("doubled way: %ld\n", timeDiff * SONIC_SPEED);
        
       
        
		//ipc to main
		if(!pthread_rwlock_wrlock(ir_args->lock)){
			perror("us_lock failed");
		}

		ir_args->timestamp = get_time_us();
		ir_args->data = &distance;
		
		if(!pthread_rwlock_unlock(ir_args->lock)){
			perror("us_lock failed");
		}

        sleep(1); //TODO: Reasonable or no sleeptime
        
    }
}

