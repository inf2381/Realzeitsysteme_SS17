

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>

#include "gpio.h"
#include "helper.h"
#include "infrared.h"
#include "common.h"




void infraredSetup() {
	GPIO_export(PIN_IR_IN1);
	GPIO_setDirection(PIN_IR_IN1, PIN_IN);

	GPIO_export(PIN_IR_IN2);
	GPIO_setDirection(PIN_IR_IN1, PIN_IN);

	GPIO_export(PIN_IR_IN3);
	GPIO_setDirection(PIN_IR_IN1, PIN_IN);

	GPIO_export(PIN_IR_IN4);
	GPIO_setDirection(PIN_IR_IN1, PIN_IN);
}


void infraredSetdown() {
	GPIO_unexport(PIN_IR_IN1);
	GPIO_unexport(PIN_IR_IN2);
	GPIO_unexport(PIN_IR_IN3);
	GPIO_unexport(PIN_IR_IN4);
}



void *infrared_read(void *arg) {
	thread_args* ir_args = (thread_args*) arg;
	

	if (VERBOSE_DEF) {
    	printf("infrared_read");
	}

	while (true) { 
		int in1 = GPIO_read(PIN_IR_IN1);
		int in2 = GPIO_read(PIN_IR_IN2);
		int in3 = GPIO_read(PIN_IR_IN3);
		int in4 = GPIO_read(PIN_IR_IN4);

		if (VERBOSE_DEF) {
			printf("infrared_read: in1 %d, in2 %d, in3 %d, in4 %d, %p \n", in1, in2, in3, in4, ir_args->lock);
		}
		
		if(pthread_rwlock_wrlock(ir_args->lock)){
			perror("ir_wrlock failed");
		}

		ir_args->timestamp = get_time_us();
		int data = 0;
		if (in1) data |= IR_IN1_BIT;
		if (in2) data |= IR_IN2_BIT;
		if (in3) data |= IR_IN3_BIT;
		if (in4) data |= IR_IN4_BIT;
		ir_args->data = &data;
		
		if(pthread_rwlock_unlock(ir_args->lock)){
			perror("ir_wrlock failed");
		}

		
		sleep(1); //TODO: Reasonable or no sleeptime
	}
}

