#ifndef killswitch_h
#define killswitch_h

#define PIN_KILLSWITCH "3"

void killswitchSetup();
void killswitchSetdown();

//Threads
void *killswitch_read(void *arg);

#endif
