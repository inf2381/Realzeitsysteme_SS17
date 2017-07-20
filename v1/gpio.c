#include "gpio.h"
#include "helper.h"

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



int verbose = 0;
const int BUFFER_SIZE = 16;

const char* PATH = "/sys/class/gpio/gpio";
const char* PATH_SUFFIX_VALUE = "/value";
const char* PATH_SUFFIX_DIR = "/direction";

const char* PATH_EXPORT = "/sys/class/gpio/export";
const char* PATH_UNEXPORT = "/sys/class/gpio/unexport";

char* GPIO_getPath(char* pin, const char* suffix) {
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


int GPIO_read(char* pin) {
	int value = -1; //default ret
	int readCount = 0;
	FILE *gpio;
	char buffer[BUFFER_SIZE];
	
	char* path = GPIO_getPath(pin, PATH_SUFFIX_VALUE);
	
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
	char* path = GPIO_getPath(pin, PATH_SUFFIX_VALUE);
	
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
	char* path = GPIO_getPath(pin, PATH_SUFFIX_DIR);
	writeSafe(path, direction);
}

