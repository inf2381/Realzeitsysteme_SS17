#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"
#include "engine.h"
#include "logic.h"

int logic_mode = -1;

char ir_state = -1;
long us_distance = -1;
int rfid_state = -1;



void logic_test_engine(){
	//left test
	engineDrive(forward, stop);
	sleep(1); 
	engineDrive(reverse, stop);
	sleep(1);

	//right test
	engineDrive(stop, forward);
	sleep(1); 
	engineDrive(stop, reverse);
	sleep(1);

	//both
	engineDrive(forward, forward);
	sleep(1); 
	engineDrive(reverse, reverse);
	sleep(1);

	engineStop();
	sleep(5);
}


void logic_test_rfid(){
	//simple rfid detectiont est: drive forward and stop if we detect a chip
	if (rfid_state == 1) {
		engineStop();
		logic_mode = none;

		if (VERBOSE_DEF) {
			printf("found chip, stopping logic");
		}
	} else {
		engineDrive(forward, forward);
	}

}

void logic_test_us(){
	//simple test: drive until we found a object
	if (us_distance < 10) {
		engineStop();
		logic_mode = none;

		if (VERBOSE_DEF) {
			printf("found obejct, stopping logic");
		}
	} else {
		engineDrive(forward, forward);
	}

}

void logic_test_ir(){
	//TODO: find a useful testcase
	

}

void logic_setup(int mode){
	logic_mode = mode;
}
void logic_shutdown(){
}
void logic_compute(){
	switch(logic_mode){
        case none:
            break;

		case track_path:
			break;
		case track_rfid_search:
			break;


		case test_ir:
			logic_test_ir();
			break;


		case test_us:
			logic_test_us();
			break;

		case test_rfid:
			logic_test_rfid();
			break;

		case test_engine:
			logic_test_engine();	

			break;
	}

}


void *exploitMeasurements(void *arg) {
    exploiterParams explparam = *(exploiterParams*) arg;
    
    while (true) {
		usleep(1000 * 1000);


        //TODO: check timestamps, maybe include trylocks
        //infrared
        if(pthread_rwlock_rdlock(explparam.ir->lock)){
            perror("ir_rdlock failed");
        }
        
        if (explparam.ir->data != NULL) {
            ir_state = *((char*) explparam.ir->data);
        }
        
        if(pthread_rwlock_unlock(explparam.ir->lock)){
            perror("ir_wrlock failed");
        }
        
        //ultrasonic
        if(pthread_rwlock_rdlock(explparam.us->lock)){
            perror("us_rdlock failed");
        }
        
        if (explparam.us->data != NULL) {
            us_distance = *((long*) explparam.us->data);
        }
        
        if(pthread_rwlock_unlock(explparam.us->lock)){
            perror("us_wrlock failed");
        }
        
        //rifd
        if(pthread_rwlock_rdlock(explparam.rfid->lock)){
            perror("rfid_rdlock failed");
        }
        
        if (explparam.rfid->data != NULL) {        
            rfid_state = *((int*) explparam.rfid->data);
        }
        
        if(pthread_rwlock_unlock(explparam.rfid->lock)){
 			perror("rifd_unlock failed");
        }
          
        
        if (VERBOSE_DEF){
            printf("collectData:: ir_state %d, us_distance %ld, rfid_state %d \n", ir_state, us_distance, rfid_state);
        }
	        
	if (ir_state == -1 || us_distance == -1 || rfid_state == -1) {
		continue;
	}
		logic_compute();
    }
}



