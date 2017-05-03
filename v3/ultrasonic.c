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

/* TODO:
 * 		- change from double to int or long (including get_time_ms)
 *		- define high and low in gpio.h
 */
long getDistance() {
	long startTime, endTime, timeDiff;

	//Trigger measurement
	GPIO_set(PIN_TRIGGER, GPIO_HIGH);

	usleep(WAIT_TO_END_TRIGGER_ys);
	GPIO_set(PIN_TRIGGER, GPIO_LOW);

	startTime = get_time_ns();
	endTime = startTime;

	while (GPIO_read(PIN_ECHO) == 0) {	 //Is this too fast for gpio?
		startTime = get_time_ns();
	}
	while (GPIO_read(PIN_ECHO) == 1) {
		endTime = get_time_ns();
	}

	timeDiff = endTime - startTime;

	return (timeDiff * SONIC_SPEED) / 2;
}



void *measureDistance(void *result) {
    
    *((long*)result) = getDistance();
    
    pthread_exit(NULL);
}


