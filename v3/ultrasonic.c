/* https://tutorials-raspberrypi.de/entfernung-messen-mit-ultraschallsensor-hc-sr04/ */

#include "ultrasonic.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>
#include "gpio.h"
#include "helper.h"

#include <stdio.h>



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
    int pipeFD = *((int *) arg);
    ssize_t resultWrite;
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
        
        
        resultWrite = write(pipeFD, &distance, __SIZEOF_LONG__);
        if (resultWrite != __SIZEOF_LONG__) {
            perror("Write");
            exit(2);
        }
        
        sleep(1); //TODO: Reasonable or no sleeptime
        
    }
}

