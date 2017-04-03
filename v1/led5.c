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

#include "helper.h"
#include "gpio.h"


const char* READ_PIN = "17";
const char* WRITE_PIN = "18";

const int SLEEPTIME = 100 * 1000; // 1/10 second
const int BLINK_HERTZ = 5;

volatile int enableBlinking = 1;
volatile int killswitch = 1;


void sig_handler(int signo)
{
	if (signo == SIGINT){
		  printf("received SIGINT, terminating\n");
		  killswitch = 0;
	}
}


int main(int argc, char** argv) {
	printf("RESY Task 1.2 5Hz Blink\n");


	if (argc > 1 && (strcmp(argv[1], "-v") == 0)){
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
		if ((value = GPIO_read((char*) READ_PIN)) > -1) {
			enableBlinking = value;           
		}
		usleep(SLEEPTIME);
	}
	GPIO_unexport((char*) READ_PIN);
	
	//wait for the read thread to cleanup & quit
	pthread_join(pth, NULL);	    
	return EXIT_SUCCESS;
}

