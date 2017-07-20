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
#ifdef TIMEMEASUREMENT  //see common.h
    struct timespec start_time = {0};
    struct timespec end_time = {0};
    long long *buffer = getTimeBuffer(BUF_SIZE);  // getBuf in helper.h; BUF_SIZE in common.h
    int current_index = 0;
#endif


    sched_setaffinity(0, sizeof(cpuset_logic), &cpuset_logic);
    thread_setPriority(PRIO_KILL);
    
    struct timespec sleeptime_kill = {0};
	

	if (VERBOSE_DEF) {
    	printf("kill_read");
	}
    
    clock_gettime( CLOCK_MONOTONIC, &sleeptime_kill );

	while (shouldRun) {
#ifdef TIMEMEASUREMENT
        clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif
	
		int killOn = GPIO_read(PIN_KILLSWITCH);
		
		if (VERBOSE_DEF) {
			//printf("kill: on %d\n", killOn);
		}
		
		if (killOn == 0) {
			printf("killswitch!\n");
            kill( getppid(), SIGINT );
			break;            		
		}

        increaseTimespec(INTERVAL_INPUT_KILL_US * NANOSECONDS_PER_MICROSECOND, &sleeptime_kill);
        
        
#ifdef TIMEMEASUREMENT
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        appendToBuf(buffer, &current_index, diff_time_ns(&start_time, &end_time));
#endif
        
        sleepAbsolute(&sleeptime_kill);

    }
    
#ifdef TIMEMEASUREMENT
    logToCSV("log_killswitch.csv", buffer);
#endif

	pthread_exit(0);
}

