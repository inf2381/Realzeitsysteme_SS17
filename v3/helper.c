#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include "common.h"

/* Common small helper file to reduce redundant code over and over again */


void enforceMalloc(void* ptr) {
	if (ptr == NULL) {
		//If malloc failes, perror failes also. A direct works. 
		char * out = "Malloc failed!\n";
		write(2, out, strlen(out));
		//perror("Malloc failed!");
		exit(EXIT_FAILURE);
	}
}

int validateInt(char* str){
	char* ep;
	long val;
	
	//Snippet from the manpage
	errno = 0;
	val = strtoul(str, &ep, 10);
 
	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
		   || (errno != 0 && val == -1)) {
	   perror("Strtol failed ");
	   exit(EXIT_FAILURE);
	}

	//ep == str --> does not start with a num
	// *ep != '\0' --> only a part of the string is a num
	//int range checks
	if (ep == str || *ep != '\0' || val > INT_MAX || val < INT_MIN) {
	   fprintf(stderr, "%s cannot be parsed to a int\n", str);
	   exit(EXIT_FAILURE);
	}
	
	return (int)val;
}

//Snippet from L3 Matrix
int countLines(char* fileName){	
	FILE* fp;
	char *line = NULL;
    size_t len = 0;
    int counterRows = 0;
    
	fp = fopen(fileName, "r");
	if (fp == NULL) {
		perror("Failed to open the inputfile");
		exit(EXIT_FAILURE);
	}
	
    while (getline(&line, &len, fp) != -1) {
        counterRows++;
    }
    
    fclose(fp);
    free(line);
    return counterRows;
}

void enforceTime(int retval){
    if (retval == -1){
        perror("timestamp");
        exit(EXIT_FAILURE);
    }
}

long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long seconds = (tv.tv_sec - 1480464000) * 1000000 ; //1.1.2017 00:00:00UTC
    return (seconds + tv.tv_usec);
}

// maybe pass pointers not copies
long diff_time_us(struct timeval before, struct timeval after)
{
    struct timeval result;
    result.tv_sec = after.tv_sec - before.tv_sec;
    result.tv_usec= after.tv_usec- before.tv_usec;
    if (result.tv_usec < 0) {
        result.tv_sec--;
        /* result->tv_usec is negative, therefore we use "+" */
        result.tv_usec = MICROSECONDS_PER_SECOND+result.tv_usec;
    }
    return result.tv_sec * MICROSECONDS_PER_SECOND + result.tv_usec;
}


long long diff_time_ns(struct timespec *before, struct timespec *after)
{
    struct timespec result;
    result.tv_sec = after->tv_sec - before->tv_sec;
    result.tv_nsec= after->tv_nsec - before->tv_nsec;
    if (result.tv_nsec < 0) {
        result.tv_sec--;
        /* result->tv_nsec is negative, therefore we use "+" */
        result.tv_nsec = NANOSECONDS_PER_SECOND+result.tv_nsec;
    }
    return result.tv_sec * NANOSECONDS_PER_SECOND + result.tv_nsec;
}


long long *getTimeBuffer(int size) {
    void * buf = (void*) malloc(size * sizeof(struct timespec));
    enforceMalloc(buf);
    return buf;
}

void appendToBuf(long long* target, long long timediff_ns) {
    static int count = 0;
    if (count <= BUF_SIZE) {  //defined in common.h
        target[count] = timediff_ns;
        count++;
    }
}

void logToCSV(const char* filename, long long* buffer) {
    printf("Logging into file: %s\n", filename);
    FILE *fp = fopen("filename", "w+");
    if (fp == NULL) {
        perror("Open file");
    }
    for (int i = 0; i<BUF_SIZE; i++) {
        int ret = fprintf(fp, "%lld, ", buffer[i]);
        if (ret < 0) {
            perror("fprintf fail");
        }
    }
    fclose(fp);
}


void activWait(int waitTimeMillis){
    double t1, t2;
    double toReach = (double)waitTimeMillis;
    t1 = get_time_us()/1000.0;
    
    //just some stuff to let time pass
    double pogo;
    while (42) {
        for (int i = 0; i < 1337; i++) {
            pogo = (i^7) / 0.13 + 42;
        }
        printf("%lf\n", pogo);
        t2 = get_time_us()/1000.0;
        if ((t2-t1) > toReach)
            break;
    }
}


void increaseTimespec(long long nanoseconds, struct timespec * sleeptime){
    long long nanos = nanoseconds + sleeptime->tv_nsec;
    sleeptime->tv_nsec = 0;
    if (nanos >= NANOSECONDS_PER_SECOND) {
        sleeptime->tv_sec += nanos / NANOSECONDS_PER_SECOND;
        sleeptime->tv_nsec += nanos % NANOSECONDS_PER_SECOND;
    } else {
        sleeptime->tv_nsec += nanos;
    }
}


void sleepAbsolute(struct timespec * sleeptime) {
    int retVal;
    
    while ((retVal = clock_nanosleep( CLOCK_MONOTONIC,
                           TIMER_ABSTIME,
                           sleeptime,
                           NULL)) == EINTR ) { };
    if (retVal != 0) {
        printf("clock_nanosleep sec %ld nsec %ld\n", sleeptime->tv_sec, sleeptime->tv_nsec );
        printf("clock_nanosleep fail: %s\n", strerror(retVal));
    }

    //sleeptime reached
    return;
}

//strupr is no part of our libc
//snippet from http://c.ittoolbox.com/groups/technical-functional/c-l/strupr-and-strlwr-are-not-working-5272245 */
void strupr (char *p) { 
	while(*p) 
	{ 
		*p=toupper(*p); 
		p++; 
	} 
}

static int rndInit = 0;
int genRandom(int randMin, int randMax) {
	if (!rndInit) {
		srand(time(NULL));
		rndInit = 1;
	}
    return (rand() % (randMax +1 - randMin) + randMin);
}

void inputNonsensical(char opt, int value) {
    printf("Your entered option makes no sense: -%c: %d\n", opt, value);
    exit(EXIT_FAILURE);
}

void printArgumentMissing(char opt) {
    printf("Option -%c requires an argument!\n", opt);
    exit(EXIT_FAILURE);
}


//@return 1 if successfull
int thread_setPriority(int priority){
	return 1;
	struct sched_param param;
	int my_pid = getpid();
	int low_priority, high_priority;

	high_priority = sched_get_priority_max(SCHED_FIFO);
	low_priority = sched_get_priority_min(SCHED_FIFO);
	
	if (high_priority < priority && priority > low_priority) {  
	    return 0;
	}

	if (sched_getparam(my_pid, &param) != 0){
		perror("sched_getparam failed");
		return 0;
	}

	param.sched_priority = priority;
	if (sched_setscheduler(my_pid, SCHED_FIFO, &param) == -1){
		perror("sched_setscheduler failed");
		return 0;
	}	
	
	return 1;	
}



