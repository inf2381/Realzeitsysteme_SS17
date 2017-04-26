/* https://tutorials-raspberrypi.de/entfernung-messen-mit-ultraschallsensor-hc-sr04/ */

#include "ultrasonic.h"

#include <stdlib.h>
#include <unistd.h>
#include "gpio.h"
#include "helper.h"



void ultrasonic_Setup() {
	GPIO_export(PIN_TRIGGER);
	GPIO_setDirection(PIN_TRIGGER, PIN_OUT);

	GPIO_export(PIN_ECHO);
	GPIO_setDirection(PIN_ECHO, PIN_IN);
}


/* TODO:
 * 		- change from double to int or long (including get_time_ms)
 *		- define high and low in gpio.h
 */
double getDistance() {
	double startTime, endTime, timeDiff;

	//Trigger measurement
	GPIO_set(PIN_TRIGGER, '1');

	usleep(WAIT_TO_END_TRIGGER_ys);
	GPIO_set(PIN_TRIGGER, '0');

	startTime = get_time_ms();
	endTime = startTime;

	while (GPIO_read(PIN_ECHO) == 1) {	 //Is this too fast for gpio?
		startTime = get_time_ms();
	}
	while (GPIO_read(PIN_ECHO) == 0) {
		endTime = get_time_ms();
	}

	timeDiff = endTime - startTime;

	return (timeDiff * SONIC_SPEED) / 2;
}
