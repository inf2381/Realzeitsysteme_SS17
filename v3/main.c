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
    piezoSetdown();

	logic_shutdown();
}


void initArgsGeneric(thread_args args, pthread_rwlock_t* lock){
	if (!pthread_rwlock_init(lock, NULL)){
		perror("genric_lock_init");
        exit(1);
    }

	args.lock = lock;
	args.timestamp = 0;
	args.data = NULL;
} 

int main(int argc, char *argv[]) {	  
	printf("RESY ROBOT - TEAM 4\n");  
	if (signal(SIGINT, sig_handler) == SIG_ERR){
        exit(EXIT_FAILURE);
    }

    setup();

    //preparing structs
	initArgsGeneric(ir_args, &ir_lock);
	initArgsGeneric(us_args, &us_lock);
	initArgsGeneric(rfid_args, &rfid_lock);
    
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
