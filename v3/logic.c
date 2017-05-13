#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"
#include "engine.h"
#include "logic.h"

int logic_mode = -1;

char ir_state;
long us_distance;
char rfid_state;

void logic_setup(int mode){
	logic_mode = mode;
}
void logic_shutdown(){
}
void logic_compute(char ir_state, long us_distance, char rfid_state){
	switch(logic_mode){
		case test_ir:
		break;
	}

}


void *exploitMeasurements(void *arg) {
    exploiterParams explparam = *(exploiterParams*) arg;
    
    while (true) {
        //TODO: check timestamps, maybe include trylocks
        //infrared
        if(!pthread_rwlock_rdlock(explparam.ir->lock)){
            perror("ir_rdlock failed");
        }
        
        ir_state = *((char*) explparam.ir->data);
        
        
        if(!pthread_rwlock_unlock(explparam.ir->lock)){
            perror("ir_wrlock failed");
        }
        
        //ultrasonic
        if(!pthread_rwlock_rdlock(explparam.us->lock)){
            perror("us_rdlock failed");
        }
        
        us_distance = *((long*) explparam.us->data);
        
        
        if(!pthread_rwlock_unlock(explparam.us->lock)){
            perror("us_wrlock failed");
        }
        
        //rifd
        rfid_state = 0;
        
        if (VERBOSE_DEF){
            printf("collectData:: ir_state %d, us_distance %ld, rfid_state %d \n", ir_state, us_distance, rfid_state);
        }
        
        // not finished yet
    }
}



