//engine.c
//ReSy SS17 -- Group 4

#include <stdio.h>
#include "helper.h"
#include "gpio.h"
#include <stdlib.h>

#define PIN_1 "5"
#define PIN_2 "6"
#define PIN_3 "13"
#define PIN_4 "19"


enum direction {
    stop = 0,
    forward = 1,
    reverse = -1
};


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


void engineDrive(int left, int right){
    
    GPIO_set(PIN_1, 1);
    

}


int main(int argc, const char * argv[]) {
    
    engineSetup();
    engineDrive(forward, forward);

    
    
    
    return EXIT_SUCCESS;
}
