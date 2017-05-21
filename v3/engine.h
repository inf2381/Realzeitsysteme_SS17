//
//  engine.h


#ifndef engine_h
#define engine_h

#define PIN_1 "5"
#define PIN_2 "6"
#define PIN_3 "13"
#define PIN_4 "19"

#define PWM_CYCLES 100

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
 * @param arg is pointer to a byte where the controling thread writes the command (see common.h)
 */
void *engineControl(void *arg)

void pwmTest();

#endif /* engine_h */
