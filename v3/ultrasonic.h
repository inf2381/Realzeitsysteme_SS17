#ifndef ultrasonic_h
#define ultrasonic_h

#define PIN_TRIGGER "18"
#define PIN_ECHO "24"
#define SONIC_SPEED 34 

#define WAIT_TO_END_TRIGGER_ys 10


void ultrasonic_Setup();
void ultrasonicSetdown();

void *measureDistance(void *arg);

#endif
