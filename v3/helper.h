#ifndef HELPER_H
#define HELPER_H
void enforceMalloc(void* ptr);
int validateInt(char* str);
int countLines(char* fileName);
long get_time_ms();
long get_time_ns();
void activWait(int waitTimeMillis);
void strupr(char* p);
int genRandom(int randMin, int randMax);
void printArgumentMissing(char opt);
void inputNonsensical(char opt, int value);
#endif

