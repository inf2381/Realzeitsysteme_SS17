#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <ctype.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

/* Common small helper file to reduce redundant code over and over again */


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

//Snippet from L3 Matrix
int countLines(char* fileName){	
	FILE* fp;
	char *line = NULL;
    size_t len = 0;
    int counterRows = 0;
    
	fp = fopen(fileName, "r");
	if (fp == NULL) {
		perror("Failed to open the inputfile");
		exit(EXIT_FAILURE);
	}
	
    while (getline(&line, &len, fp) != -1) {
        counterRows++;
    }
    
    fclose(fp);
    free(line);
    return counterRows;
}

void enforceTime(int retval){
    if (retval == -1){
        perror("timestamp");
        exit(EXIT_FAILURE);
    }
}

//timmurphy.org/2009/10/18/millisecond-timer-in-c-c/
double get_time_ms(){
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec + (t.tv_usec / 1000000.0)) * 1000.0;
}

long get_time_ns() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    
    return t.tv_sec * 1000000000 + t.tv_nsec;
}

void activWait(int waitTimeMillis){
    double t1, t2;
    double toReach = (double)waitTimeMillis;
    t1 = get_time_ms();
    
    //just some stuff to let time pass
    double pogo;
    while (42) {
        for (int i = 0; i < 1337; i++) {
            pogo = (i^7) / 0.13 + 42;
        }
        printf("%lf\n", pogo);
        t2 = get_time_ms();
        if ((t2-t1) > toReach)
            break;
    }
}

//strupr is no part of our libc
//snippet from http://c.ittoolbox.com/groups/technical-functional/c-l/strupr-and-strlwr-are-not-working-5272245 */
void strupr (char *p) { 
	while(*p) 
	{ 
		*p=toupper(*p); 
		p++; 
	} 
}

static int rndInit = 0;
int genRandom(int randMin, int randMax) {
	if (!rndInit) {
		srand(time(NULL));
		rndInit = 1;
	}
    return (rand() % (randMax +1 - randMin) + randMin);
}

void inputNonsensical(char opt, int value) {
    printf("Your entered option makes no sense: -%c: %d\n", opt, value);
    exit(EXIT_FAILURE);
}

void printArgumentMissing(char opt) {
    printf("Option -%c requires an argument!\n", opt);
    exit(EXIT_FAILURE);
}

