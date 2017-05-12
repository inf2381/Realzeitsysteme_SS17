/* engine.c */
/* ReSy SS17 -- Group 4 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "engine.h"
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

void changeMovement(char* pin_1, char* pin_2, int direction){
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

void engineDrive(int left, int right){
    changeMovement(PIN_1, PIN_2, left);
    changeMovement(PIN_3, PIN_4, right);
}

