//test stop file
#include "common.h"
#include "engine.h"

//dummy vars
volatile engineMode engineCtrl; //see common.h
volatile int shouldRun = 1;     //see common.h
cpu_set_t cpuset_logic;     //see common.h
cpu_set_t cpuset_sensors;   //see common.h
cpu_set_t cpuset_engine;    //see common.h
int default_degree = 90;
int default_logicmode = 0;

int main(int argc, char *argv[]) {	
    engineSetdown();
}
