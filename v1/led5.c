//
//  led5.c
//
//
//  
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

#include <pthread.h>

const int BUFFER_SIZE = 16;

const char* PATH = "/sys/class/gpio/gpio";
const char* PATH_SUFFIX = "/value";

const char* PATH_EXPORT = "/sys/class/gpio/export";
const char* PATH_UNEXPORT = "/sys/class/gpio/unexport";

const char* READ_PIN = "17";
const char* WRITE_PIN = "18";

const int SLEEPTIME = 100 * 1000; // 1/10 second
const int BLINK_HERTZ = 5;

volatile int enableBlinking = 1;


void enforceMalloc(void* ptr) {
    if (ptr == NULL) {
        //If malloc failes, perror failes also. A direct call works.
        char * out = "Malloc failed!\n";
        write(2, out, sizeof("Malloc failed!\n"));
        //perror("Malloc failed!");
        exit(EXIT_FAILURE);
    }
}


char* getValuePath(char* pin) {
	 //preparing for open
    int sizeConcat = strlen(PATH) + strlen(pin) + strlen(PATH_SUFFIX);
    
    char* concatPath = (char*) malloc(sizeConcat + 1); //nullbyte
    enforceMalloc(concatPath);
    sprintf(concatPath, "%s%s%s", PATH, pin, PATH_SUFFIX);
    
    return concatPath;
}

void writeSafe(char* path, char* value){
	FILE *gpio;
    gpio = fopen(concatPath, "w");
    if (gpio != NULL){

		if (fwrite(&strValue, sizeof(char), 1, gpio) != 0){
			perror("fwrite failed");
			exit(EXIT_FAILURE);
		}

		if (fclose(gpio) = 0) {
			perror("fclose failed");
			exit(EXIT_FAILURE);
		}
    } else {
		perror("fopen failed");
		exit(EXIT_FAILURE);
    }
}


int readGPIO(char* pin) {
    int value = -1; //default ret
    FILE *gpio;
    char buffer[BUFFER_SIZE];
	
	char* path = getValuePath(pin);
    
    gpio = fopen(path, "r");
    if (gpio != NULL){
        if (fread(buffer, BUFFER_SIZE, 1, gpio) != 0) {
			perror("fwrite failed");
			exit(EXIT_FAILURE);
		}
		
        value = buffer[0] - '0'; 
		
		//value validty check
		if (value != 0 && value != 1){
			printf("Value is unexpected. Buffer: %s\n", buffer);
			exit(EXIT_FAILURE);
		}

		if (fclose(gpio) != 0) {
			perror("fclose failed");
			exit(EXIT_FAILURE);
		}
    } else {
		perror("fopen failed");
		exit(EXIT_FAILURE);
    }
    
    
    return value;
}

void setGPIO(char* pin, char value) {
    char* path = getValuePath(pin);
    
	char strValue = value + '0';   
    writeSafe(concatPath, strValue);
}


void exportGPIO(char* pin) {
	writeSafe(PATH_EXPORT, pin)
}

void unexportGPIO(char* pin) {
	writeSafe(PATH_UNEXPORT, pin)
}


void *threadFunc(void *arg)
{
	int isRunning = *((int*) arg);
	int blink_sleep = (1000 * 1000) / BLINK_HERTZ;
	int value = 1;

	exportGPIO(WRITE_PIN);
    while (isRunning) {
		//value = (~value) & 1
		if (value == 1) {
			value = 0;
		} else {
			value = 1;
		}
			
		if (enableBlinking == 1) {
			 setGPIO(WRITE_PIN, value);
		}
			  
        
        usleep(blink_sleep);
    }
    
    unexportGPIO(WRITE_PIN);

	return NULL;
}



int main() {
	//TODO: SIGKILL abfangen und killswitch auf 0 setzten
	
	int killswitch = 1;
	
	pthread_t pth;	
	pthread_create(&pth, NULL, threadFunc, &killswitch);
	
    int oldValue = 1; //default value on rasp pi
    int numberOfPushes = 0;
    int val;
    
    while (killswitch) {
        if ((val = readGPIO(READ_PIN)) > -1) {
			enableBlinking = val;           
        }
        usleep(SLEEPTIME);
    }
    
    
    return EXIT_SUCCESS;
}

