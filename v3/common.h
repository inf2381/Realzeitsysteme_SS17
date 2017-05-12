#ifndef common_h
#define common_h

#define VERBOSE_DEF 1 
#define MOVE_ENABLED 0


typedef struct {
    pthread_rwlock_t* lock;

	long timestamp;
	void* data;
} thread_args;



#define IR_IN1_BIT 0x01
#define IR_IN2_BIT 0x02
#define IR_IN3_BIT 0x04
#define IR_IN4_BIT 0x08


#endif
