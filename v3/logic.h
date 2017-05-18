#ifndef logic_h
#define logic_h

void logic_setup(int mode);
void logic_shutdown();
void logic_compute(char ir_state, long us_distance, char rfid_state);
void *exploitMeasurements(void *arg);

enum logic_mode {
	none = 0;
	track_path = 1,
	track_rfid_search = 2,
	test_ir = 3,
	test_us = 4,
	test_rfid = 5,
	test_engine = 6

};

#endif
