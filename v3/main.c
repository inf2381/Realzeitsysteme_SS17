#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "engine.h"
#include "ultrasonic.h"


void sig_handler(int signo)
{
    if (signo == SIGINT){
        engineStop();
        engineSetdown();
	ultrasonicSetdown();
        exit(EXIT_SUCCESS);
    }
}


void setUp() {

	engineSetup();
    ultrasonic_Setup();

}


int main(int argc, char *argv[]) {
	long dist;
	setUp();

	if (signal(SIGINT, sig_handler) == SIG_ERR){
        	exit(EXIT_FAILURE);
    }
    
	while (true) {
		dist = getDistance();
		printf("Distance: %ld\n", dist);
		if(dist < 8000000) {
			engineStop();
			continue;;
		}
		//engineDrive(forward, forward);
		sleep(1);

	}
	engineSetdown();
	ultrasonicSetdown();
	return EXIT_SUCCESS;
}
