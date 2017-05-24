#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <getopt.h>
#include <string.h>

#include "common.h"
#include "engine.h"
#include "ultrasonic.h"
#include "infrared.h"
#include "logic.h"
#include "piezo.h"
#include "rfid.h"
#include "helper.h"

pthread_t   thread_us, thread_ir, thread_rfid, thread_exploit, thread_engine;
pthread_rwlock_t ir_lock, us_lock, rfid_lock;
thread_args ir_args, us_args, rfid_args;
exploiterParams explParam;
volatile engineMode engineCtrl;  //see common.h

int logicmode = track_path;


void setup() {
	engineSetup();
    ultrasonicSetup();
	infraredSetup();
    piezoSetup();
    rfidSetup();

	logic_setup(logicmode);
    engineCtrl = STAY;
printf("stay %d, left %d\n", STAY, ONLY_LEFT);
}

void shutdown(){
    logic_shutdown();

	engineSetdown();
	ultrasonicSetdown();
	infraredSetdown();
    piezoSetdown();
}


void initArgsGeneric(thread_args* args, pthread_rwlock_t* lock){
	if (pthread_rwlock_init(lock, NULL)) {
		perror("genric_lock_init");
        exit(1);
    }

	args->lock = lock;
	args->timestamp = 0;
	args->data = NULL;
} 

void sig_handler(int signo)
{
    if (signo == SIGINT){
        shutdown();
        exit(EXIT_SUCCESS);
    }
}

void readCommandLine(int argc, char *argv[]){
     while (42) {
        static struct option long_options[] = {
            {"mode", required_argument, 0, 'm'},
            {NULL, 0, 0, 0}
        };
        int option_index = 0;
        int option = getopt_long_only(argc, argv, "m:", long_options, &option_index);

        if (option == -1)
            break;

        switch(option){
        case 'm':
            if (strcmp(optarg, "rfid") == 0) {
                logicmode = test_rfid;

            } else if (strcmp(optarg, "ir") == 0) {
                logicmode = test_ir;

            } else if (strcmp(optarg, "us") == 0) {
                logicmode = test_us;

            } else if (strcmp(optarg, "piezo") == 0) {
                logicmode = test_piezo;

            } else if (strcmp(optarg, "engine") == 0) {
                logicmode = test_engine;

            } else if (strcmp(optarg, "path") == 0) {
                logicmode = track_path;

            } else if (strcmp(optarg, "search") == 0) {
                logicmode = track_rfid_search;
            }
            break;
 
        case ':':
            if(optopt == 'e' || optopt == 'E')
                break;
            printArgumentMissing(optopt);
            break;
        case '?':
            break;
         default:
            break;
        }
    }
}


int main(int argc, char *argv[]) {	  
	printf("RESY ROBOT - TEAM 4\n");  
	if (signal(SIGINT, sig_handler) == SIG_ERR){
        exit(EXIT_FAILURE);
    }

    readCommandLine(argc, argv);
    setup();

/*    
    while (true) {
        struct timeval startTime, endTime;
        struct timespec pogo;
        gettimeofday(&startTime, NULL);
        sleepAbsolute(100000, &pogo);
        gettimeofday(&endTime, NULL);
        
        printf("Absolute expected: %d total %ld\n", 100, diff_time_us(startTime, endTime));
        sleep(1);
    }
*/
    //preparing structs
	initArgsGeneric(&ir_args, &ir_lock);
	initArgsGeneric(&us_args, &us_lock);
	initArgsGeneric(&rfid_args, &rfid_lock);
    
    explParam.ir = &ir_args;
    explParam.us = &us_args;
    explParam.rfid = &rfid_args;
    explParam.engineControl = &engineCtrl;
	printf("%p val %d\n", &engineCtrl, engineCtrl);
    
    //starting threads
    pthread_create(&thread_us, NULL, measureDistance, (void*) &us_args);
	pthread_create(&thread_ir, NULL, infrared_read, (void*) &ir_args);
    pthread_create(&thread_rfid, NULL, detectRFID, (void*) &rfid_args);
    pthread_create(&thread_exploit, NULL, exploitMeasurements, (void*) &explParam);
    pthread_create(&thread_engine, NULL, engineController, (void*) &engineCtrl);
    
    //wait for exploiting thread to finish
    pthread_join(thread_exploit, NULL);

	shutdown();
	return EXIT_SUCCESS;
}
