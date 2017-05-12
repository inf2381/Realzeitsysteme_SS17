#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"
#include "engine.h"
#include "logic.h"

int logic_mode = -1;

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



