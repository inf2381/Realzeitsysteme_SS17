
#ifndef HELPER_H
#define HELPER_H
#include <sys/time.h>

#define MICROSECONDS_PER_SECOND 1000000
#define NANOSECONDS_PER_SECOND (long long) 1000000000
#define NANOSECONDS_PER_MILLISECOND 1000000
#define NANOSECONDS_PER_MICROSECOND 1000

void enforceMalloc(void* ptr);
int validateInt(char* str);
int countLines(char* fileName);
long get_time_us();
long diff_time_us(struct timeval before, struct timeval after);
long long diff_time_ns(struct timespec *before, struct timespec *after);
long long *getTimeBuffer(int size);  //allocates heap for an timediff array of the given size
void appendToBuf(long long* target, long long timediff_ns);  //appends until buffer full
void logToCSV(const char* filename, long long* buffer);
void activWait(int waitTimeMillis);
void increaseTimespec(long long nanoseconds, struct timespec * sleeptime);
void sleepAbsolute(struct timespec * sleeptime);
void strupr(char* p);
int genRandom(int randMin, int randMax);
void printArgumentMissing(char opt);
void inputNonsensical(char opt, int value);
int thread_setPriority(int priority);
#endif

