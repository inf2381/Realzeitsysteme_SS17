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


void setup() {

	engineSetup();
    ultrasonic_Setup();

}


int main(int argc, char *argv[]) {
	int ultasonicPipeFD[2], createPipe;
    pthread_t   thread_US_write, thread_US_read;
    
	setup();
    //additional setup
    createPipe = pipe(ultasonicPipeFD);
    if (createPipe < 0){
        perror("pipe ");
        exit(1);
    }
    
	if (signal(SIGINT, sig_handler) == SIG_ERR){
        	exit(EXIT_FAILURE);
    }
    
    //starting threads
    pthread_create(&thread_US_read,NULL,exploitDistance,&ultasonicPipeFD[0]);
    pthread_create(&thread_US_write,NULL,measureDistance,&ultasonicPipeFD[1]);
    
	while (true) {
		
        engineDrive(forward, forward);
		sleep(1);

	}
	engineSetdown();
	ultrasonicSetdown();
	return EXIT_SUCCESS;
}
