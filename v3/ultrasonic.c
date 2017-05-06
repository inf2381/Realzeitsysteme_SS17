/* https://tutorials-raspberrypi.de/entfernung-messen-mit-ultraschallsensor-hc-sr04/ */

#include "ultrasonic.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
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


long getDistance() {
	long startTime, endTime, timeDiff;

	//Trigger measurement
	GPIO_set(PIN_TRIGGER, GPIO_HIGH);

	usleep(WAIT_TO_END_TRIGGER_ys);
	GPIO_set(PIN_TRIGGER, GPIO_LOW);

	startTime = get_time_ms();
	endTime = startTime;

	while (GPIO_read(PIN_ECHO) == 0) {
        startTime = get_time_ms();
	}
	while (GPIO_read(PIN_ECHO) == 1) {
		endTime = get_time_ms();
	}

	timeDiff = endTime - startTime;

	return (timeDiff * SONIC_SPEED) / 2;
}



void *measureDistance(void *result) {
    
    *((long*)result) = getDistance();
    
    pthread_exit(NULL);
}


