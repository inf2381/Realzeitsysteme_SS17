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
int reverseEnabled = 0;


struct timespec timer_now = {0};
struct timespec timer_endtime = {0};


const long long  NANOSECONDS_PER_DEGREE = NANOSECONDS_PER_MILLISECOND * 13;
const int US_TRIGGER_THRESHOLD = 30 * 1000;
const int REVERT_TIMEOUT_NS = NANOSECONDS_PER_MILLISECOND * 50;

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
    sleep(3);

    engineCtrl = PWM_LEFT;
    sleep(3);
    
    engineCtrl = PWM_RIGHT;
    sleep(3);

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
        engineCtrl = PWM_75;
	}

}

void logic_test_us(){
	//simple test: drive until we found a object
	if (us_distance < US_TRIGGER_THRESHOLD) {
        engineCtrl = STAY;
		logic_mode = none;

		if (VERBOSE_DEF) {
			printf("found object, stopping logic");
		}
	} else {
        engineCtrl = PWM_75;
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
            if (right_inner || right_outer) {
                ir_test_state = detect_left;
                engineCtrl = PWM_LEFT;

                printf("right --> left\n");
            }               

            break;

        case detect_left:
            if (left_inner || left_outer) {
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


int helper_isTimerFinished(){
    clock_gettime(CLOCK_MONOTONIC, &timer_now);
    return (timer_now.tv_sec >= timer_endtime.tv_sec && timer_now.tv_nsec >= timer_endtime.tv_nsec);
}


//check if robot turns around
//@return 1 if robot moves
int turnCheck(){
     if (turnLeftEnabled || turnRightEnabled) {
   
        if (helper_isTimerFinished()){
            //endstate reached
            if (turnLeftEnabled) {
                turnLeftEnabled = 0;
                return 0;
            }
            
            if (turnRightEnabled) {
                turnRightEnabled = 0;
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
    long long timeDiff = NANOSECONDS_PER_DEGREE * degree;
	printf("timediff %lld ms\n", timeDiff / NANOSECONDS_PER_MILLISECOND);
    clock_gettime(CLOCK_MONOTONIC, &timer_endtime);
    
    increaseTimespec(timeDiff, &timer_endtime);
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

void logic_test_turn(){
    if (turnLeftEnabled || turnRightEnabled) {
        if (!turnCheck()) {
		    printf("turn end\n");
            engineCtrl = STAY;
            sleep(5);
        }
    } else {
        turnLeft(default_degree);
    }
}


void logic_path(){
    /* possible strategy:
    drive fast straight until curve, slow down on first ir detection, 
    correction: by one motor for x ms (find a angle-time forumla), afterwards drive at 25%

    */
    
    if (path_state == path_start) {        
        
        //RFID
        if (rfid_state == 1) {
            //PATH EXIT
            //TODO: how to ensure that actual rfid is not ending search? inter
            logic_mode = track_rfid_search;
            printf("Exit path");
            return;
        }
        
        //is reverting?
        if (reverseEnabled) {
            if (helper_isTimerFinished()) {
                reverseEnabled = 0;
                if (turnRightEnabled) {
                    turnRight();
                }else if (turnLeftEnabled) {
                    turnLeft();
                }
                printf("reverse end\n");
            } else {
                return;
            }
        }
        
        //already turning?
        if (turnLeftEnabled || turnRightEnabled) {
            if (turnCheck() == 0) {
                printf("turn end\n");
            } else {
                return;
            }
        }
        
        
        //IR
        char right_outer = ir_state & IR_IN2_BIT;
        char right_inner = ir_state & IR_IN1_BIT;

        char left_inner = ir_state & IR_IN3_BIT;
        char left_outer = ir_state & IR_IN4_BIT;
        


        const int CORRECTION_ANGLE = 30;

        if ((right_inner || right_outer) && (left_inner || left_outer)) {
	       engineCtrl = STOP; //100msec revert
	       
	       reverseEnabled = 1;
	       clock_gettime(CLOCK_MONOTONIC, &timer_endtime);
           increaseTimespec(REVERT_TIMEOUT_NS , &timer_endtime);
           
           return;
	    } else if (right_inner || right_outer) {
            turnRightEnabled = 1;
            return;
        } else if (left_inner || left_outer) {
            turnLeftEnabled = 1;
            return;
        } 

        engineCtrl = PWM_75;
    }
}

void logic_rfid_search(){
	const int speed = FULL_THROTTLE;

       if (rfid_state == 1) {
            //EXIT
            //TODO: how to ensure that actual rfid is not ending search? inter
            logic_mode = none;
		engineCtrl = STAY;
            return;
        }



     if (turnLeftEnabled || turnRightEnabled) {
        if (turnCheck() == 0) {
            printf("turn end\n");
            engineCtrl = speed;
            
         }
         return;
     }

    if (us_distance < US_TRIGGER_THRESHOLD) {
        turnLeft(90);
		printf("turn");		
	} else {
        engineCtrl = speed;
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
		    logic_rfid_search();
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
			
	    case test_turn:
	        logic_test_turn();	
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



