#ifndef common_h
#define common_h
#include <pthread.h>

#define VERBOSE_DEF 1
#define VERBOSE_LOG_GPIO_DEF 0 
#ifdef __arm__
    #define GPIO_ENABLED 1
#else
    #define GPIO_ENABLED 0
#endif

typedef enum {
    STAY,
    STOP,
    FULL_THROTTLE,
    PWM_25,
    PWM_50,
    PWM_75,
    REVERSE,
    ONLY_LEFT,
    ONLY_RIGHT,
    PWM_LEFT,
    PWM_RIGHT
} engineMode;

typedef struct {
    pthread_rwlock_t* lock;

	long timestamp;
	void* data;
} thread_args;

typedef struct {
    thread_args *ir;
    thread_args *us;
    thread_args *rfid;
} exploiterParams;

//Used in ir_args.data as bitflags
#define IR_IN1_BIT 0x01
#define IR_IN2_BIT 0x02
#define IR_IN3_BIT 0x04
#define IR_IN4_BIT 0x08

//Timings
#define INTERVAL_LOGIC 100
#define INTERVAL_INPUT 50

#endif
