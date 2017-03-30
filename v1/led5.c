//
//  switch.c
//
//
//  Created by Marc Bormeth on 29.03.17.
//
//

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

const int PATHSIZE = 21;
const char* PATH = "/sys/class/gpio/gpio";
const char* PATH_SUFFIX = "/value";
const int PIN = 17;
const int SLEEPTIME = 100 * 1000; // 1/10 second


void enforceMalloc(void* ptr) {
    if (ptr == NULL) {
        //If malloc failes, perror failes also. A direct works.
        char * out = "Malloc failed!\n";
        write(2, out, sizeof("Malloc failed!\n"));
        //perror("Malloc failed!");
        exit(EXIT_FAILURE);
    }
}

int readGPIO(int pin) {
    int value = -1; //default ret
       
    //preparing for open
    int sizeConcat = strlen(PATH) + 1 + strlen(PATH_SUFFIX);
    
    if (pin >= 10)
        sizeConcat++;
    
    char* concatPath = (char*) malloc(sizeConcat + 1); //nullbyte
    enforceMalloc(concatPath);
    sprintf(concatPath, "%s%d%s", PATH, pin, PATH_SUFFIX);

    //opening and reading Pin
    FILE *gpio;
    char buffer[10];
    
    gpio = fopen(concatPath, "r");
    if (gpio != NULL){
        fread(buffer, 10, 1, gpio);

        value = buffer[0] - '0'; 
	if (value != 0 && value != 1){
		printf("Value is unexpected. Buffer: %s\n", buffer);
		exit(EXIT_FAILURE);
	}

	fclose(gpio);
    } else {
	perror("fopen failed");
	exit(EXIT_FAILURE);
    }
    
    
    return value;
}



int main() {
    int isRunning = 1;
    int oldValue = 1; //default value on rasp pi
    int numberOfPushes = 0;
    int val;
    
    while (isRunning) {
        if ((val = readGPIO(PIN)) > -1) {
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
    
    
    return EXIT_SUCCESS;
}
