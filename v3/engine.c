/* engine.c */
/* ReSy SS17 -- Group 4 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "engine.h"
#include "common.h"
#include "helper.h"
#include "gpio.h"
#include "piezo.h"


void engineSetup(){
    if (VERBOSE_DEF) {
	    printf("Engine::PWM_CYCLES %d\n", PWM_CYCLES);
	}
    GPIO_export(PIN_1);
    GPIO_setDirection(PIN_1, PIN_OUT);
    
    GPIO_export(PIN_2);
    GPIO_setDirection(PIN_2, PIN_OUT);
    
    GPIO_export(PIN_3);
    GPIO_setDirection(PIN_3, PIN_OUT);
    
    GPIO_export(PIN_4);
    GPIO_setDirection(PIN_4, PIN_OUT);
}

void engineSetdown(){
    shouldRun = 0;

    GPIO_unexport(PIN_1);
    GPIO_unexport(PIN_2);
    GPIO_unexport(PIN_3);
    GPIO_unexport(PIN_4);  
}

void engineStop(){
	//Fast stopping, short reverse phase
    GPIO_set(PIN_1, 0);
    GPIO_set(PIN_2, 1);
    GPIO_set(PIN_3, 0);
    GPIO_set(PIN_4, 1);

    usleep(100000);
    GPIO_set(PIN_1, 0);
    GPIO_set(PIN_2, 0);
    GPIO_set(PIN_3, 0);
    GPIO_set(PIN_4, 0);
}

/* if the car isn't moving, the short reverse phase from engine stop would make the car move */
void allPinsToZero() {
    GPIO_set(PIN_1, 0);
    GPIO_set(PIN_2, 0);
    GPIO_set(PIN_3, 0);
    GPIO_set(PIN_4, 0);
}

void changeMovement(char* pin_1, char* pin_2, int direction) {
    switch (direction) {
        case forward:
            GPIO_set(pin_1, 1);
            GPIO_set(pin_2, 0);
            break;
            
        case stop:
            GPIO_set(pin_1, 0);
            GPIO_set(pin_2, 0);
            break;
            
        case reverse:
            GPIO_set(pin_1, 0);
            GPIO_set(pin_2, 1);
            
            break;
        default:
            engineStop();
            engineSetdown();
            printf("You can't fly with a pi! Look at the enum direction %d", direction);
            exit(EXIT_FAILURE);
    }
    
}

void engineDrive(int left, int right) {
	//TODO: skip writing if the actual state matches the requested state

    changeMovement(PIN_1, PIN_2, left);
    changeMovement(PIN_3, PIN_4, right);
}


// PWM functions

void pwmTest() {
    struct timespec sleeper, dummy;
    sleeper.tv_sec = 0;
    sleeper.tv_nsec = PWM_LOW_TIME_NS;
    
    GPIO_set(PIN_1, 1);
    
    int i;
    for (i=0; i<100; i++) {
        GPIO_set(PIN_3, 1);
        nanosleep(&sleeper, &dummy);
        GPIO_set(PIN_3, 0);
        nanosleep(&sleeper, &dummy);
    }
    GPIO_set(PIN_1, 0);
}

void pwmDrive(char *leftPin, char *rightPin, struct timespec *hightime, struct timespec *downtime) {
    int i;
    
    
    if (leftPin == NULL) {
        for (i = 0; i < PWM_CYCLES; i++) {
            GPIO_set(rightPin, 1);
            clock_nanosleep(CLOCK_MONOTONIC,
                            0,
                            hightime,
                            NULL);
            GPIO_set(rightPin, 0);
            clock_nanosleep(CLOCK_MONOTONIC,
                            0,
                            downtime,
                            NULL);
        }
        return;
    }
    
    if (rightPin == NULL) {
        for (i = 0; i < PWM_CYCLES; i++) {
            GPIO_set(leftPin, 1);
            clock_nanosleep(CLOCK_MONOTONIC,
                            0,
                            hightime,
                            NULL);
            GPIO_set(leftPin, 0);
            clock_nanosleep(CLOCK_MONOTONIC,
                            0,
                            downtime,
                            NULL);
        }
        return;

    }
    
    for (i = 0; i < PWM_CYCLES; i++) {
        GPIO_set(leftPin, 1);
        GPIO_set(rightPin, 1);
        clock_nanosleep(CLOCK_MONOTONIC,
                        0,
                        hightime,
                        NULL);
        GPIO_set(leftPin, 0);
        GPIO_set(rightPin, 0);
        clock_nanosleep(CLOCK_MONOTONIC,
                        0,
                        downtime,
                        NULL);
    }
}

void *engineController(void *arg) {
#ifdef TIMEMEASUREMENT  //see common.h
    struct timespec start_time = {0};
    struct timespec end_time = {0};
    long long *buffer = getTimeBuffer(BUF_SIZE);  // getBuf in helper.h; BUF_SIZE in common.h
    int current_index = 0;
#endif
    
    sched_setaffinity(0, sizeof(cpuset_engine),&cpuset_engine);
    thread_setPriority(PRIO_ENGINE);
    
    struct timespec sleep_high = {0}; //needed for sleeping absolutely within pwm
    struct timespec sleep_down = {0};
    struct timespec sleeptime_engine = {0};
    int fun = 0;
    
    sleep_high.tv_nsec = HIGH_75_NS;
    sleep_down.tv_nsec = LOW_75_NS;
    
    clock_gettime( CLOCK_MONOTONIC, &sleeptime_engine );
    while (shouldRun) {
#ifdef TIMEMEASUREMENT
        clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif
        if (fun == 1 && engineCtrl != REVERSE) {
            fun = 0;
            piezo_stopReverse();
        }
        if(VERBOSE_DEF) {
            printf("engineCtrl %d\n", engineCtrl);
        }
        switch (engineCtrl) {
            case STAY:
                allPinsToZero();
                break;
            case STOP:
                engineStop();
                break;
            case FULL_THROTTLE:
                GPIO_set(PIN_1, 1);
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_3, 1);
                GPIO_set(PIN_4, 0);
                break;
            case PWM_75:
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_4, 0);
                pwmDrive(PIN_1, PIN_3, &sleep_high, &sleep_down);
                break;
            case REVERSE:
                GPIO_set(PIN_1, 0);
                GPIO_set(PIN_2, 1);
                GPIO_set(PIN_3, 0);
                GPIO_set(PIN_4, 1);
                
                if (FUN_ENABLED_DEF) {
                    piezo_playReverse();
                    fun = 1;
                }
                break;
            case ONLY_LEFT:
                GPIO_set(PIN_1, 1);
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_3, 0);
                GPIO_set(PIN_4, 1);
                break;
            case ONLY_RIGHT:
                GPIO_set(PIN_1, 0);
                GPIO_set(PIN_2, 1);
                GPIO_set(PIN_3, 1);
                GPIO_set(PIN_4, 0);
                break;
            case PWM_LEFT:
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_3, 0);
                GPIO_set(PIN_4, 0);
                pwmDrive(PIN_1, NULL, &sleep_high, &sleep_down);
                break;
            case PWM_RIGHT:
                GPIO_set(PIN_1, 0);
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_4, 0);
                pwmDrive(NULL, PIN_3, &sleep_high, &sleep_down);
                break;
            default:
                allPinsToZero();
                break;
        }
        
        increaseTimespec(SLEEPTIME_NS, &sleeptime_engine);
        sleepAbsolute(&sleeptime_engine);
        
#ifdef TIMEMEASUREMENT
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        appendToBuf(buffer, &current_index, diff_time_ns(&start_time, &end_time));
#endif
    }
    
#ifdef TIMEMEASUREMENT
    logToCSV("log_engine.csv", buffer);
#endif
    pthread_exit(0);
    
}















