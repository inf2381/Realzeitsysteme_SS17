#ifndef common_h
#define common_h

#include <pthread.h>
#include <sched.h>

#define VERBOSE_DEF 0
#define VERBOSE_LOG_GPIO_DEF 0
#define FUN_ENABLED_DEF 0
#ifdef __arm__
    #define GPIO_ENABLED 1
#else
    #define GPIO_ENABLED 0
#endif
#define LOAD_KERNELMODULE_DEF 0

#define TIMEMEASUREMENT
#define BUF_SIZE 10000  //needed for logging the elapsed time

typedef enum {
    STAY,
    STOP,
    FULL_THROTTLE,
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

//Defined in main.c, used to terminate the thread loops
extern volatile int shouldRun;

//Also defined in main.c
extern cpu_set_t cpuset_logic;
extern cpu_set_t cpuset_sensors;
extern cpu_set_t cpuset_engine;
extern int default_degree;
extern int default_logicmode;

//Used in ir_args.data as bitflags
#define IR_IN1_BIT 0x01
#define IR_IN2_BIT 0x02
#define IR_IN3_BIT 0x04
#define IR_IN4_BIT 0x08

//Timings
#define INTERVAL_INPUT_KILL_US 1000
#define INTERVAL_LOGIC_US 25000
#define INTERVAL_INPUT_US 30000
#define INTERVAL_ULTRASONIC_US (50 * NANOSECONDS_PER_MILLISECOND)

//Thresholds
#define MEASUREMENT_EXPIRATION_US 1000 * 1000  // us

// Scheduling stuff
// 0 is reserved for IRQs and the rest of the OS
#define CPU_LOGIC 1
#define CPU_SENSORS 2
#define CPU_ENGINE 3

//prio
#define PRIO_KILL -11
#define PRIO_LOGIC -10
#define PRIO_SENSORS -8
#define PRIO_ENGINE -9

#endif
