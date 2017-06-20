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

int rfidCounter;

int rfidHistory[RFID_FIND_COUNT];

struct timespec timer_now = {0};
struct timespec timer_endtime = {0};


const long long  NANOSECONDS_PER_DEGREE = NANOSECONDS_PER_MILLISECOND * 4;
const int US_TRIGGER_THRESHOLD = 30 * 1000;
const int REVERT_TIMEOUT_NS = NANOSECONDS_PER_MILLISECOND * 200;

const int RFID_FOUND_TIMEOUT_US = 1000 * 500;

const int CORRECTION_ANGLE = 40;


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
	if (rfid_state > 0) {  //rfid_state could also be -1 or 42 at the moment
	    if (VERBOSE_DEF) {
            printf("%d\n", rfid_state);
        }
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
	//simple test: drive until we found an object
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
    
    helper_turnComputeDegree(degree);
    
    engineCtrl = ONLY_LEFT;
    turnLeftEnabled = 1;
}

void turnRight(int degree){
    helper_turnComputeDegree(degree);
    
    engineCtrl = ONLY_RIGHT;
    turnRightEnabled = 1;
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


void startReverse() {
    engineCtrl = REVERSE;
    reverseEnabled = 1;
    clock_gettime(CLOCK_MONOTONIC, &timer_endtime);
    increaseTimespec(REVERT_TIMEOUT_NS , &timer_endtime);
}

void helper_addRfidChipToHistory(){
    rfidHistory[rfidCounter] = rfid_state;
    rfidCounter++;
}

//@return 1 if new
int helper_isRfidChipNew(){
    for (int i = 0; i < rfidCounter; i++) {
        if (rfidHistory[i] == rfid_state) {
            return 0;
        }
    }
    
    return 1;
}

void logic_path(){
    /* possible strategy:
    drive fast straight until curve, slow down on first ir detection, 
    correction: by one motor for x ms (find a angle-time forumla), afterwards drive at 25%

    */
    
    if (path_state == path_start) {        
        
        //RFID
        if (rfid_state  == RFID_EXIT_UID) {
            //PATH EXIT
            helper_addRfidChipToHistory();
            logic_mode = track_rfid_search;
            if (VERBOSE_DEF) {
                printf("Found chip with Uid-sum: %d. Exit path\n", rfid_state);
            }
            return;
        } else {
             if (VERBOSE_DEF) {
                printf("Found non-exit chip with Uid: %d. expectd %d \n", rfid_state, RFID_EXIT_UID);
            }
        }
        
        //is reverting?
        if (reverseEnabled) {
            if (helper_isTimerFinished()) {
                reverseEnabled = 0;
                if (VERBOSE_DEF) {
                    printf("reverse end\n");
                }
                
                if (turnRightEnabled) {
                    turnRight(CORRECTION_ANGLE);
                }else if (turnLeftEnabled) {
                    turnLeft(CORRECTION_ANGLE);
                }
            } else {
                return;
            }
        }
        
        //already turning?
        if (turnLeftEnabled || turnRightEnabled) {
            if (turnCheck() == 0) {
                if (VERBOSE_DEF) {
                    printf("turn end\n");
                }
            } else {
                return;
            }
        }
        
        
        //IR
        char right_outer = ir_state & IR_IN2_BIT;
        char right_inner = ir_state & IR_IN1_BIT;

        char left_inner = ir_state & IR_IN3_BIT;
        char left_outer = ir_state & IR_IN4_BIT;
        

        if ((right_inner || right_outer) && (left_inner || left_outer)) {
            engineCtrl = STOP; //100msec reverse
            startReverse();
            turnLeftEnabled = 1;
            return;
	    } else if (right_inner || right_outer) {
            startReverse();
            turnRightEnabled = 1;
            return;
        } else if (left_inner || left_outer) {
            startReverse();
            turnLeftEnabled = 1;
            return;
        } 

        engineCtrl = PWM_75;
    }
}

void logic_rfid_search(){
	const int speed = FULL_THROTTLE;
	const int random_degree_min = 90;
	const int random_degree_max = 180;
	
	

    if (rfid_state > 0 && helper_isRfidChipNew()) {
        helper_addRfidChipToHistory();

        if (1) {
            printf("rfid_search, found chip %d, uid %d\n", rfidCounter, rfid_state);
        }  
        
        engineCtrl = STAY;
        usleep(RFID_FOUND_TIMEOUT_US);
        
        if (rfidCounter == 4) {
            // mission completed
            engineCtrl = STAY;
            logic_mode = none;
        }
        return;
    }
   
    
    if (turnLeftEnabled || turnRightEnabled) {
        if (turnCheck() == 0) {
            if (VERBOSE_DEF) {
                printf("turn end\n");
            }
            engineCtrl = speed;
        }
        return;
    }

    if (us_distance < US_TRIGGER_THRESHOLD) {
        int leftOrRight = genRandom(0, 1);
        int degree = genRandom(random_degree_min, random_degree_max);
        
        if (leftOrRight == 0) {
            //left
            turnLeft(degree);
            
        } else {
            //right
            turnRight(degree);
        }
    
        if (VERBOSE_DEF) {
        	printf("turn leftOrRight %d, degree %d \n", leftOrRight, degree);	
        }
		
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


/**
 * Checks, if the timestamp violates a threshold.
 * Thread exits with current logicmode to pass status to main thread
 */
void helper_checkTimestamp(long *current, long *toCheck, const char* source) {
    
    if (VERBOSE_DEF) {
        printf("timediff %ld\n", (*current -*toCheck));        
    }

    // Threshold defined in common.h
    if ((*current - *toCheck) > MEASUREMENT_EXPIRATION_US && toCheck != 0) {
        printf("threshold - timediff %ld, source %s\n", (*current -*toCheck), source);  
        engineCtrl = STAY;
        default_logicmode = logic_mode;
        pthread_exit(NULL);
    }
}


void *exploitMeasurements(void *arg) {
    sched_setaffinity(0, sizeof(cpuset_logic),&cpuset_logic);
    thread_setPriority(PRIO_LOGIC);

    exploiterParams explparam = *(exploiterParams*) arg;
    struct timespec sleeptime_logic = {0};
    long time_now = 0;  // needed to check timestamps from the measuring threads
    
    clock_gettime(CLOCK_MONOTONIC, &sleeptime_logic);
    while (logic_mode != none) {
        time_now = get_time_us();
        
        //TODO: maybe include trylocks
        //infrared
        if(pthread_rwlock_rdlock(explparam.ir->lock)){
            perror("ir_rdlock failed");
        }
        
        if (explparam.ir->data != NULL) {
            helper_checkTimestamp(&time_now, &explparam.ir->timestamp, "ir");
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
            //helper_checkTimestamp(&time_now, &explparam.us->timestamp, "us");
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
            helper_checkTimestamp(&time_now, &explparam.rfid->timestamp, "rfid");
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



