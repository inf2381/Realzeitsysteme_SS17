#ifndef HELPER_H
#define HELPER_H
#include <sys/time.h>

#define MICROSECONDS_PER_SECOND 1000000
#define NANOSECONDS_PER_SECOND 1000000000
#define NANOSECONDS_PER_MILLISECOND 1000000

void enforceMalloc(void* ptr);
int validateInt(char* str);
int countLines(char* fileName);
long get_time_us();
long diff_time_us( struct timeval before, struct timeval after );
void activWait(int waitTimeMillis);
void sleepAbsolute(int nanoseconds, struct timespec * sleeptime);
void strupr(char* p);
int genRandom(int randMin, int randMax);
void printArgumentMissing(char opt);
void inputNonsensical(char opt, int value);
#endif

