#ifndef piezo_h
#define piezo_h

#define PIEZO_PIN "22"

void piezoSetup();
void piezoSetdown();
void playTone();

void piezo_playReverse();
void piezo_stopReverse();

#endif
