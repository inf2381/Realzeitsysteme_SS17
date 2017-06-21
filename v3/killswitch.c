#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>
#include <signal.h>

#include "gpio.h"
#include "helper.h"
#include "killswitch.h"
#include "common.h"



void killswitchSetup() {
	GPIO_export(PIN_KILLSWITCH);
	GPIO_setDirection(PIN_KILLSWITCH, PIN_IN);
}


void killswitchSetdown() {
	GPIO_unexport(PIN_KILLSWITCH);
}



void *killswitch_read(void *arg) {
    sched_setaffinity(0, sizeof(cpuset_sensors), &cpuset_sensors);
    thread_setPriority(PRIO_KILL);
    
    struct timespec sleeptime_kill = {0};
	

	if (VERBOSE_DEF) {
    	printf("kill_read");
	}
    
    clock_gettime( CLOCK_MONOTONIC, &sleeptime_kill );

	while (shouldRun) {
		int killOn = GPIO_read(PIN_KILLSWITCH);
		
		if (VERBOSE_DEF) {
			//printf("kill: on %d\n", killOn);
		}
		
		if (killOn == 1) {
            kill( getppid(), SIGINT );            		
		}

        increaseTimespec(INTERVAL_INPUT_KILL_US * NANOSECONDS_PER_MICROSECOND, &sleeptime_kill);
        sleepAbsolute(&sleeptime_kill);
    }

	pthread_exit(0);
}

