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
#include <signal.h>

const int BUFFER_SIZE = 16;

const char* PATH = "/sys/class/gpio/gpio";
const char* PATH_SUFFIX_VALUE = "/value";
const char* PATH_SUFFIX_DIR = "/direction";

const char* PATH_EXPORT = "/sys/class/gpio/export";
const char* PATH_UNEXPORT = "/sys/class/gpio/unexport";

const char* READ_PIN = "17";
const char* WRITE_PIN = "18";

const int SLEEPTIME = 100 * 1000; // 1/10 second
const int BLINK_HERTZ = 5;

volatile int enableBlinking = 1;
volatile int killswitch = 1;

int verbose = 0;

void enforceMalloc(void* ptr) {
	if (ptr == NULL) {
		//If malloc failes, perror failes also. A direct call works.
		char * out = "Malloc failed!\n";
		write(2, out, sizeof("Malloc failed!\n"));
		//perror("Malloc failed!");
		exit(EXIT_FAILURE);
	}
}


char* getGPIOPath(char* pin, const char* suffix) {
	 //preparing for open
	int sizeConcat = strlen(PATH) + strlen(pin) + strlen(suffix);
	
	char* concatPath = (char*) malloc(sizeConcat + 1); //nullbyte
	enforceMalloc(concatPath);
	sprintf(concatPath, "%s%s%s", PATH, pin, suffix);
	
	return concatPath;
}

void writeSafe(char* path, char* value){
	if (verbose) printf("writeSafe path %s value %s\n", path, value);
	
	FILE *gpio;
	gpio = fopen(path, "w");
	if (gpio != NULL){
		if (fwrite(value, sizeof(char), strlen(value), gpio) != strlen(value)){
			perror("fwrite failed");
			exit(EXIT_FAILURE);
		}

		if (fflush(gpio) != 0){
			perror("fflush fail");
		}

		if (fclose(gpio) != 0) {
			perror("writeSafe: fclose failed");
			exit(EXIT_FAILURE);
		}
	} else {
		perror("writeSafe: fopen failed");
		exit(EXIT_FAILURE);
	}
}


int readGPIO(char* pin) {
	int value = -1; //default ret
	int readCount = 0;
	FILE *gpio;
	char buffer[BUFFER_SIZE];
	
	char* path = getGPIOPath(pin, PATH_SUFFIX_VALUE);
	
	gpio = fopen(path, "r");
	if (gpio != NULL){
		if ((readCount = fread(buffer, 1, BUFFER_SIZE, gpio)) != 2) {
			perror("fread failed");
			printf("cnt %d\n", readCount);
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

void GPIO_set(char* pin, char value) {
	char* path = getGPIOPath(pin, PATH_SUFFIX_VALUE);
	
	char strValue = value + '0';
	char buffer[2] = { strValue, 0 };   
	writeSafe(path, (char*) buffer);
}


void GPIO_export(char* pin) {
	writeSafe((char*) PATH_EXPORT, pin);
}

void GPIO_unexport(char* pin) {
	writeSafe((char*)PATH_UNEXPORT, pin);
}

void GPIO_setDirection(char* pin, char* direction){
	char* path = getGPIOPath(pin, PATH_SUFFIX_DIR);
	writeSafe(path, direction);
}


void *threadFunc(void *arg)
{
	int blink_sleep = (1000 * 1000) / BLINK_HERTZ;
	int value = 1;

	GPIO_export((char*) WRITE_PIN);
	GPIO_setDirection((char*) WRITE_PIN, (char*) "out");
	while (killswitch) {
		//value = (~value) & 1
		if (value == 1) {
			value = 0;
		} else {
			value = 1;
		}
			
		if (enableBlinking == 1) {
			 GPIO_set((char*) WRITE_PIN, value);
		}
			  
		
		usleep(blink_sleep);
	}
	
	GPIO_unexport((char*) WRITE_PIN);
	return NULL;
}

void sig_handler(int signo)
{
	if (signo == SIGINT){
		  printf("received SIGINT, terminating\n");
		  killswitch = 0;
	}
}


int main(int argc, char** argv) {
	printf("RESY Task 1.2 5Hz Blink\n");
	
	if (argc == 2 && argv[1] == "-v"){
		verbose = 1;
	}
	
	if (signal(SIGINT, sig_handler) == SIG_ERR){
		 printf("Can't catch SIGINT\n");
	 }
	
	pthread_t pth;	
	pthread_create(&pth, NULL, threadFunc, NULL);


	GPIO_export((char*) READ_PIN);
	while (killswitch) {
		int value;
		if ((value = readGPIO((char*) READ_PIN)) > -1) {
			enableBlinking = value;           
		}
		usleep(SLEEPTIME);
	}
	GPIO_unexport((char*) READ_PIN);
	
	//wait for the read thread to cleanup & quit
	pthread_join(pth, NULL);	    
	return EXIT_SUCCESS;
}

