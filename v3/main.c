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
#include "piezo.h"


pthread_t   thread_us, thread_ir, thread_exploit;
pthread_rwlock_t ir_lock, us_lock, rfid_lock;
thread_args ir_args, us_args, rfid_args;
exploiterParams explParam;


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
    piezoSetup();

	logic_setup(test_ir);
}

void shutdown(){
	engineSetdown();
	ultrasonicSetdown();
	infrared_Setdown();
    void piezoSetdown();
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
    if (!pthread_rwlock_init(&us_lock, NULL)){
        perror("ir_lock");
        exit(1);
    }
    if (!pthread_rwlock_init(&rfid_lock, NULL)){
        perror("ir_lock");
        exit(1);
    }

    //preparing structs
    ir_args.lock = &ir_lock;
    ir_args.timestamp = 0;
    ir_args.data = NULL;
    
    us_args.lock = &us_lock;
    us_args.timestamp = 0;
    us_args.data = NULL;
    
    rfid_args.lock = &rfid_lock;
    rfid_args.timestamp = 0;
    rfid_args.data = NULL;
    
    
    explParam.ir = &ir_args;
    explParam.us = &us_args;
    explParam.rfid = &rfid_args;

    
    //starting threads
    pthread_create(&thread_us, NULL, measureDistance, (void*) &us_args);
	pthread_create(&thread_ir, NULL, infrared_read, (void*) &ir_args);
    pthread_create(&thread_exploit, NULL, exploitMeasurements, (void*) &explParam);
    
    //wait for exploiting thread to finish
    pthread_join(thread_exploit, NULL);

	shutdown();
	return EXIT_SUCCESS;
}
