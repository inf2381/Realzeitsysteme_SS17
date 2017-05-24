/* engine.c */
/* ReSy SS17 -- Group 4 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#include "engine.h"
#include "common.h"
#include "helper.h"
#include "gpio.h"

void engineSetup(){
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

void pwmDrive(char *leftPin, char *rightPin, long hightime, long downtime, struct timespec *sleeper) {
    int i;
    
    for (i = 0; i < PWM_CYCLES; i++) {
        GPIO_set(leftPin, 1);
        GPIO_set(rightPin, 1);
        sleepAbsolute(hightime, sleeper);
        GPIO_set(leftPin, 0);
        GPIO_set(rightPin, 0);
        sleepAbsolute(downtime, sleeper);
    }
}

void *engineController(void *arg) {
    engineMode *mode = ((engineMode *) arg); //pointer to volatile variable where the direction is stored
    struct timespec sleep; //needed for sleeping absolutely within pwm
    
    
    while (true) {

	printf("engine: mode %d \n", *mode);
        switch (*mode) {
            case STAY:
                allPinsToZero();
            case STOP:
                engineStop();
                break;
            case FULL_THROTTLE:
                GPIO_set(PIN_1, 1);
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_3, 1);
                GPIO_set(PIN_4, 0);
                break;
            case PWM_25:
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_4, 0);
                pwmDrive(PIN_1, PIN_3, HIGH_25_NS, LOW_25_NS, &sleep);
                break;
            case PWM_50:
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_4, 0);
                pwmDrive(PIN_1, PIN_3, HIGH_50_NS, LOW_50_NS, &sleep);
                break;
            case PWM_75:
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_4, 0);
                pwmDrive(PIN_1, PIN_3, HIGH_75_NS, LOW_75_NS, &sleep);
                break;
            case REVERSE:
                GPIO_set(PIN_1, 0);
                GPIO_set(PIN_2, 1);
                GPIO_set(PIN_3, 0);
                GPIO_set(PIN_4, 1);
                break;
            case ONLY_LEFT:
                GPIO_set(PIN_1, 1);
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_3, 0);
                GPIO_set(PIN_4, 0);
                break;
            case ONLY_RIGHT:
                GPIO_set(PIN_1, 0);
                GPIO_set(PIN_2, 0);
                GPIO_set(PIN_3, 1);
                GPIO_set(PIN_4, 0);
                break;
            default:
		allPinsToZero();
		break;
        }
        
        sleepAbsolute(SLEEPTIME_NS, &sleep);
    }
    
}















