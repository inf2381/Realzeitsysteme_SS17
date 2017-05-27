//
//  engine.h


#ifndef engine_h
#define engine_h
#include "common.h"

#define PIN_1 "5"
#define PIN_2 "6"
#define PIN_3 "13"
#define PIN_4 "19"

#define PWM_CYCLES 100
#define SLEEPTIME_NS 250000000  //0.25 s
#define PWM_LOW_TIME_NS 10000000 // only for the pwm test; will be deleted soon

//TODO: Reasonable times

// PWM times so that the program doesn't have to calculate the time during runtime
#define HIGH_25_NS 5000000
#define LOW_25_NS 15000000
#define HIGH_50_NS 10000000
#define LOW_50_NS 10000000
#define HIGH_75_NS 15000000
#define LOW_75_NS 5000000

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
