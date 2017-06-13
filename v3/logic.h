#ifndef logic_h
#define logic_h
#include "common.h"

void logic_setup(int mode);
void logic_shutdown();
void logic_compute();
void *exploitMeasurements(void *arg);
extern volatile engineMode engineCtrl;

enum logic_mode {
	none = 0,
	track_path = 1,
	track_rfid_search = 2,
	test_ir = 3,
	test_us = 4,
	test_rfid = 5,
	test_engine = 6,
    test_piezo = 7,
    count

};


enum ir_test_states {
    ir_none,
    detect_right,
    detect_left

};


enum logic_path_states {
    path_start,
    path_slower,

};
#endif
