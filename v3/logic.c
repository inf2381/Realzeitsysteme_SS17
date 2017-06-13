#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"
#include "engine.h"
#include "logic.h"
#include "helper.h"
#include "piezo.h"

int logic_mode = -1;

char ir_state = -1;
long us_distance = -1;
int rfid_state = -1;

int ir_test_state = none;
int path_state = path_start; 

int turnLeftEnabled = 0;
int turnRightEnabled  = 0;
struct timespec turn_now = {0};
struct timespec turn_endtime = {0};

void logic_test_engine(){
	//left test
    engineCtrl = ONLY_LEFT;
	sleep(1);

	//right test
    engineCtrl = ONLY_RIGHT;
	sleep(1);

	//both
    engineCtrl = FULL_THROTTLE;
	sleep(1);

	engineCtrl = REVERSE;
	sleep(1);

    // PWM test
    engineCtrl = PWM_75;
    sleep(1);

    engineCtrl = PWM_50;
    sleep(1);

    engineCtrl = PWM_25;
    sleep(1);

    engineCtrl = PWM_LEFT;
    sleep(1);
    
    engineCtrl = PWM_RIGHT;
    sleep(1);

    //Stap it
    engineCtrl = STOP;
	sleep(1);

    engineCtrl = STAY;
    sleep(3);
}


void logic_test_rfid(){
	//simple rfid detectiont est: drive forward and stop if we detect a chip
	if (rfid_state == 1) {
        engineCtrl = STAY;
		logic_mode = none;

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
    

    char right_outer = ir_state & IR_IN2_BIT;
    char right_inner = ir_state & IR_IN1_BIT;

    char left_inner = ir_state & IR_IN3_BIT;
    char left_outer = ir_state & IR_IN4_BIT;


    
    switch(ir_test_state){
        case ir_none:
            ir_test_state = detect_right;
            engineCtrl = PWM_RIGHT;

            printf("none --> dr\n");
            break;

        case detect_right:
	//printf("Right inner: %d, Right outer %d\n", right_inner, right_outer);
            if (!right_inner || !right_outer) {
                ir_test_state = detect_left;
                engineCtrl = PWM_LEFT;

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

void logic_test_piezo(){
    playTone();
    sleep(1);

    piezo_playReverse();
    sleep(3);
    piezo_stopReverse();
    logic_mode = none;
}



//check if robot turns around
//@return 1 if robot moves
int turnCheck(){
     if (turnLeftEnabled || turnRightEnabled) {
        clock_gettime(CLOCK_MONOTONIC, &turn_now);
        
	
        if (turn_now.tv_sec >= turn_endtime.tv_sec && turn_now.tv_nsec >= turn_endtime.tv_nsec){
            //endstate reached
            if (turnLeftEnabled) {
                turnLeftEnabled = 0;
                engineCtrl = STAY;
                return 0;
            }
            
            if (turnRightEnabled) {
                turnRightEnabled = 0;
                engineCtrl = STAY;
                return 0;
                
            }
                    
        } else {
            //still in turn
            return 1;
        }
     }
     
     return 0;
}

void helper_turnComputeDegree(int degree) {
//TODO: find coorect value
    long long nanosecs_per_degree = NANOSECONDS_PER_MILLISECOND * 5;
    long long timeDiff = nanosecs_per_degree * degree;
	printf("timediff %lld ms\n", timeDiff / NANOSECONDS_PER_MILLISECOND);
    clock_gettime(CLOCK_MONOTONIC, &turn_endtime);
    
    increaseTimespec(timeDiff, &turn_endtime);
}

void turnLeft(int degree){
    //goal: compute degrees to a time value 
    
    //override protection
    if (!turnLeftEnabled) {
        helper_turnComputeDegree(degree);
        
        engineCtrl = PWM_LEFT;
        turnLeftEnabled = 1;
    } 
}

void turnRight(int degree){
    //override protection
    if (!turnRightEnabled) {
        helper_turnComputeDegree(degree);
        
        engineCtrl = PWM_RIGHT;
        turnRightEnabled = 1;
    } 
}


void logic_path(){
    /* possible strategy:
    drive fast straight until curve, slow down on first ir detection, 
    correction: by one motor for x ms (find a angle-time forumla), afterwards drive at 25%

    */

    if (turnLeftEnabled || turnRightEnabled) {
        if (!turnCheck()) {
		printf("turn end\n");
            sleep(5);
        }
    } else {
        turnLeft(90);
    }
    
    
    if (path_state == path_start) {
        //ir detection
        //rfid on --> search mode
        
        
    }
}


void logic_setup(int mode){
	logic_mode = mode;
}
void logic_shutdown(){
    logic_mode = none;    
}
void logic_compute(){
	switch(logic_mode){
        case none:
            break;

		case track_path:
            logic_path();
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

        case test_piezo:
            logic_test_piezo();	

			break;
	}

}


void *exploitMeasurements(void *arg) {
    sched_setaffinity(0, sizeof(cpuset_logic),&cpuset_logic);

    exploiterParams explparam = *(exploiterParams*) arg;
    struct timespec sleeptime_logic = {0};
    
    clock_gettime(CLOCK_MONOTONIC, &sleeptime_logic);
    while (logic_mode != none) {
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
		
		increaseTimespec(INTERVAL_LOGIC_US * NANOSECONDS_PER_MICROSECOND, &sleeptime_logic);
        sleepAbsolute(&sleeptime_logic);
    }
    
    pthread_exit(0);
}



