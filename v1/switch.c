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
const char* PATH = "/sys/class/gpio";
const int PIN = 17;
const int SLEEPTIME = 100000; // 1/10 second


void enforceMalloc(void* ptr) {
    if (ptr == NULL) {
        //If malloc failes, perror failes also. A direct works.
        char * out = "Malloc failed!\n";
        write(2, out, sizeof("Malloc failed!\n"));
        //perror("Malloc failed!");
        exit(EXIT_FAILURE);
    }
}

int validateInt(char* str){
    char* ep;
    long val;
    
    //Snippet from the manpage
    errno = 0;
    val = strtoul(str, &ep, 10);
    
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == -1)) {
        perror("Strtol failed ");
        exit(EXIT_FAILURE);
    }
    
    //ep == str --> does not start with a num
    //*ep != '\0' --> only a part of the string is a num
    //int range checks
    if (ep == str || *ep != '\0' || val > INT_MAX || val < INT_MIN) {
        fprintf(stderr, "%s cannot be parsed to a int\n", str);
        exit(EXIT_FAILURE);
    }
    
    return (int)val;
}

int readGPIO(int pin) {
    int value = -1; //default ret
    
    
    //preparing for open
    int sizeConcat = strlen(PATH) + 1;
    
    if (pin >= 10)
        sizeConcat++;
    
    char* concatPath = (char*) malloc(sizeConcat + 1); //nullbyte
    enforceMalloc(concatPath);
    sprintf(concatPath, "%s%d%s", PATH, pin, "/value\n");
    
    //opening and reading Pin
    FILE *gpio;
    char buffer[10];
    
    gpio = fopen(concatPath, O_RDONLY);
    if (gpio != NULL){
        fread(buffer, 10, 1, gpio);
        value = validateInt(buffer);
    }
    fclose(gpio);
    
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
                    printf("Pushes: %d", numberOfPushes);
                }
            }
            
        }
        usleep(SLEEPTIME);
        
    }
    
    
    return EXIT_SUCCESS;
}
