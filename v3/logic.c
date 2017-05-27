#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"
#include "engine.h"
#include "logic.h"
#include "helper.h"
#include "piezo.h"

struct timespec sleeptime;
int logic_mode = -1;

char ir_state = -1;
long us_distance = -1;
int rfid_state = -1;

int ir_test_state = none;
bool isRunning = true;
//int path_state = start; 

void logic_test_engine(){
	//left test
    engineCtrl = ONLY_LEFT;
	sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

	//right test
    engineCtrl = ONLY_RIGHT;
	sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

	//both
    engineCtrl = FULL_THROTTLE;
	sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

	engineCtrl = REVERSE;
	sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

    // PWM test
    engineCtrl = PWM_75;
    sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

    engineCtrl = PWM_50;
    sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

    engineCtrl = PWM_25;
    sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

    //Stap it
    engineCtrl = STOP;
	sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

    engineCtrl = STAY;
	sleepAbsolute(3 * NANOSECONDS_PER_SECOND, &sleeptime);
}


void logic_test_rfid(){
	//simple rfid detectiont est: drive forward and stop if we detect a chip
	if (rfid_state == 1) {
        engineCtrl = STAY;
		logic_mode = none;
        isRunning = false;

		if (VERBOSE_DEF) {
			printf("found chip, stopping logic");
		}
	} else {
        engineCtrl = PWM_50;
	}

}

void logic_test_us(){
	//simple test: drive until we found a object
	if (us_distance < 15 * 1000) {
        engineCtrl = STAY;
		logic_mode = none;
        isRunning = false;

		if (VERBOSE_DEF) {
			printf("found object, stopping logic");
		}
	} else {
        engineCtrl = FULL_THROTTLE;
    }

}

void logic_test_ir(){
    //order (right to left): 2, 1, 3, 4)
    //one line between, drive right, wait for detection on the right, drive left, wait to detection, loop
    
    while(true) {
        char right_outer = ir_state & IR_IN2_BIT;
        char right_inner = ir_state & IR_IN1_BIT;

        char left_inner = ir_state & IR_IN3_BIT;
        char left_outer = ir_state & IR_IN4_BIT;


        
        switch(ir_test_state){
            case ir_none:
                ir_test_state = detect_right;
                engineCtrl = ONLY_RIGHT;

                printf("none --> dr\n");
                break;

            case detect_right:
		//printf("Right inner: %d, Right outer %d\n", right_inner, right_outer);
                if (!right_inner || !right_outer) {
                    ir_test_state = detect_left;
                    engineCtrl = ONLY_LEFT;

                    printf("right --> left\n");
                }               

                break;

            case detect_left:
                if (!left_inner || !left_outer) {
                    ir_test_state = none;
                    printf("left --> none\n");
                }    

                break;

        }
    }
	
}

void logic_test_piezo(){
    playTone();
    sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);

    piezo_playReverse();
    sleepAbsolute(3 * NANOSECONDS_PER_SECOND, &sleeptime);
    piezo_stopReverse();
    logic_mode = none;
    isRunning = false;
}



void logic_path(){
    /* possible strategy:
    drive fast straight until curve, slow down on first ir detection, 
    correction: by one motor for x ms (find a angle-time forumla), afterwards drive at 25%

    */



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

        case test_piezo:
            logic_test_piezo();	

			break;
	}

}


void *exploitMeasurements(void *arg) {
    exploiterParams explparam = *(exploiterParams*) arg;
    
    while (isRunning) {
        sleepAbsolute(INTERVAL_LOGIC * NANOSECONDS_PER_MILLISECOND, &sleeptime);


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
        
        //rfid
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
    pthread_exit(0);
}



