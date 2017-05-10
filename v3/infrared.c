#include "infrared.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>
#include "gpio.h"
#include "helper.h"

#include <stdio.h>



void infrared_Setup() {
	GPIO_export(PIN_IR_IN1);
	GPIO_setDirection(PIN_IR_IN1, PIN_IN);

	GPIO_export(PIN_IR_IN2);
	GPIO_setDirection(PIN_IR_IN1, PIN_IN);

	GPIO_export(PIN_IR_IN3);
	GPIO_setDirection(PIN_IR_IN1, PIN_IN);

	GPIO_export(PIN_IR_IN4);
	GPIO_setDirection(PIN_IR_IN1, PIN_IN);
}


void infrared_Setdown() {
	GPIO_unexport(PIN_IR_IN1);
	GPIO_unexport(PIN_IR_IN2);
	GPIO_unexport(PIN_IR_IN3);
	GPIO_unexport(PIN_IR_IN4);
}



void *infrared_read(void *arg) {
	if (VERBOSE_DEF) {
    	printf("infrared_read");
	}

	while (true) { 
		int in1 = GPIO_read(PIN_IR_IN1);
		int in2 = GPIO_read(PIN_IR_IN2);
		int in3 = GPIO_read(PIN_IR_IN3);
		int in4 = GPIO_read(PIN_IR_IN4);

		if (VERBOSE_DEF) {
			printf("infrared_read: in1 %d, in2 %d, in3 %d, in4 %d", in1, in2, in3, in4);
		}
		
		
		
		sleep(1); //TODO: Reasonable or no sleeptime
	}
}

