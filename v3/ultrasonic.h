#ifndef ultrasonic_h
#define ultrasonic_h

#define PIN_TRIGGER "18"
#define PIN_ECHO "24"
#define SONIC_SPEED 34 

#define WAIT_TO_END_TRIGGER_US 10


void ultrasonicSetup();
void ultrasonicSetdown();

void *measureDistance(void *arg);

#endif
