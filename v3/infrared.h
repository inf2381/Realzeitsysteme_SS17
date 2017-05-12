#ifndef infrared_h
#define infrared_h

#define PIN_IR_IN1 "12"
#define PIN_IR_IN2 "16"
#define PIN_IR_IN3 "20"
#define PIN_IR_IN4 "21"

void infrared_Setup();
void infrared_Setdown();

//Threads
void *infrared_read(void *arg);

#endif
