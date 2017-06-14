//
//  engine.h


#ifndef engine_h
#define engine_h
#include "common.h"

#define PIN_1 "5"
#define PIN_2 "6"
#define PIN_3 "13"
#define PIN_4 "19"

// PWM times so that the program doesn't have to calculate the time during runtime
#define HIGH_75_NS 1500000
#define LOW_75_NS 500000


#define SLEEPTIME_NS (10 * 1000000)  //100 ms
#if (SLEEPTIME_NS%(HIGH_75_NS+LOW_75_NS) != 0)
#error Sleeptime not multiple of one pwm-cycle
#endif
#define PWM_CYCLES (SLEEPTIME_NS/(HIGH_75_NS+LOW_75_NS))


#define PWM_LOW_TIME_NS 10000000 // only for the pwm test; will be deleted soon

//TODO: Reasonable times

extern volatile engineMode engineCtrl;

enum direction {
    stop = 0,
    forward = 1,
    reverse = -1
};

/** Exports the predefined GPIO pins.
 *  First function to call if you want to start the race
 */
void engineSetup();


/** Counterpart to engineSetdown.
 *  Call it after you have won the race
 */
void engineSetdown();


/** Function to control the engines.
 * As parameters, please use values from the enum above
 */
void engineDrive(int left, int right);


// Stops both wheels immediately
void engineStop();

// Stops without a short reverse phase
void allPinsToZero();

/** Thread function to control the engine by a seperated 
 * @param arg is pointer to a variable of type engineMode,
 *  where the controling thread writes the command (see common.h)
 */
void *engineController(void *arg);

void pwmTest();

#endif /* engine_h */
