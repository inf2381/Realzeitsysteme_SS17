//
//  switch.c
//
//
//  Created by Marc Bormeth on 29.03.17.
//
//

#include "helper.h"
#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

const char* READ_PIN = 17;
const int SLEEPTIME = 100 * 1000; // 1/10 second


int main() {
    int isRunning = 1;
    int oldValue = 1; //default value on rasp pi
    int numberOfPushes = 0;
    int val;
    
    GPIO_export((char*) READ_PIN);
    while (isRunning) {
        if ((val = GPIO_read((char*) READ_PIN)) > -1) {
            if (val != oldValue ) {
                oldValue = val;
                if (val == 0) {
                    numberOfPushes++;
                    printf("Pushes: %d\n", numberOfPushes);
                }
            }
        }
        usleep(SLEEPTIME);
        
    }
    
    GPIO_unexport((char*) READ_PIN);
    
    return EXIT_SUCCESS;
}
