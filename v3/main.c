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
#include "logic.h"


pthread_t   thread_US_write, thread_US_read, thread_ir;
pthread_rwlock_t ir_lock, US_lock, rfid_lock;
thread_args ir_args, US_args, rfid_args;

char ir_state;
long us_distance;
char rfid_state;

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

	logic_setup(test_ir);
}

void shutdown(){
	engineSetdown();
	ultrasonicSetdown();
	infrared_Setdown();
}

void collectData(){
	//central collection point, should pass data to a logic unit

	//TODO: check timestamps, maybe include trylocks
	//infrared
	if(!pthread_rwlock_rdlock(ir_args.lock)){
		perror("ir_rdlock failed");
	}

	ir_state = *((char*) ir_args.data);


	if(!pthread_rwlock_unlock(ir_args.lock)){
		perror("ir_wrlock failed");
	}

	//ultrasonic
	if(!pthread_rwlock_rdlock(US_args.lock)){
		perror("us_rdlock failed");
	}

	us_distance = *((long*) US_args.data);


	if(!pthread_rwlock_unlock(US_args.lock)){
		perror("us_wrlock failed");
	}

	//rifd
	rfid_state = 0;

	if (VERBOSE_DEF){
		printf("collectData:: ir_state %d, us_distance %ld, rfid_state %d \n", ir_state, us_distance, rfid_state);
	}
	
}


int main(int argc, char *argv[]) {	    
	if (signal(SIGINT, sig_handler) == SIG_ERR){
        	exit(EXIT_FAILURE);
    }

	setup();

	//sync stuff 
	if (!pthread_rwlock_init(&ir_lock, NULL)){
		perror("ir_lock");
        exit(1);
	}	

	ir_args.lock = &ir_lock;
	ir_args.timestamp = 0;
	ir_args.data = NULL;
	
    
    //starting threads
    //pthread_create(&thread_US_read,NULL,exploitDistance, (void*) &US_args);
    pthread_create(&thread_US_write,NULL,measureDistance, (void*) &US_args);
	pthread_create(&thread_ir,NULL,infrared_read, (void*) &ir_args);
    
	while (true) {
		collectData();
		logic_compute(ir_state, us_distance, rfid_state);

	
		//defined in commom.h
		//TODO: remove this, movement should be handeled over logic.c
		if (MOVE_ENABLED) {
        	engineDrive(forward, forward);
		}

		

		sleep(1);

	}

	shutdown();
	return EXIT_SUCCESS;
}
