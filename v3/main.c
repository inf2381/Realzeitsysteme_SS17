#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "common.h"
#include "engine.h"
#include "ultrasonic.h"
#include "infrared.h"



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
	infrared_Setup();
}

void shutdown(){
	engineSetdown();
	ultrasonicSetdown();
	infrared_Setdown();
}


int main(int argc, char *argv[]) {
	int ultasonicPipeFD[2], createPipe;
    pthread_t   thread_US_write, thread_US_read, thread_ir;
	pthread_rwlock_t ir_lock, US_lock, rfid_lock;
	thread_args ir_args, US_args, rfid_args;
	long data_stub;
    
	    
	if (signal(SIGINT, sig_handler) == SIG_ERR){
        	exit(EXIT_FAILURE);
    }

	setup();

	//sync stuff
 	createPipe = pipe(ultasonicPipeFD);
    if (createPipe < 0){
        perror("pipe ");
        exit(1);
    }
 
	if (!pthread_rwlock_init(&ir_lock, NULL)){
		perror("ir_lock");
        exit(1);
	}	

	ir_args.lock = &ir_lock;
	ir_args.data = &data_stub;
	
    
    //starting threads
    pthread_create(&thread_US_read,NULL,exploitDistance,&ultasonicPipeFD[0]);
    pthread_create(&thread_US_write,NULL,measureDistance,&ultasonicPipeFD[1]);
	pthread_create(&thread_ir,NULL,infrared_read, (void*) &ir_args);
    
	while (true) {
		//defined in commom.h
		if (MOVE_ENABLED) {
        	engineDrive(forward, forward);
		}
		sleep(1);

	}

	shutdown();
	return EXIT_SUCCESS;
}
