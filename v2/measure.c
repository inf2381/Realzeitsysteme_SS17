#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <getopt.h>
#include <math.h>
#include <linux/sched.h>
#include <sched.h>

#include "helper.h"


#define defaultSleepMin 1000
#define defaultSleepMax 10000
#define defaultStep 1000
#define defaultLoop 100

int sleep_min = defaultSleepMin;
int sleep_max = defaultSleepMax;
int loop_count = defaultLoop;
int step = defaultStep;

int verbose = 0;
int enable_rt = 0;
char* outfile = NULL;
long *valueArray;
	

void parseOptions(int argc, char** argv){
	int option ;

	while (42) {
        static struct option long_options[] = {
            {"h", no_argument, 0, 'h'},
            {"v", no_argument, 0, 'v'},
            {"min", required_argument, 0, 'a'},
            {"max", required_argument, 0, 'b'},
            {"loop", required_argument, 0, 'l'},
            {"step", required_argument, 0, 's'},
            {"out", required_argument, 0, 'o'},
            {"rt", no_argument, 0, 'r'},
            {NULL, 0, 0, 0}
        };
        int option_index = 0;
        option = getopt_long_only(argc, argv, "a:b:l:s:o:rvh", long_options, &option_index);

        if (option == -1)
            break;

        switch(option){
		case 'h':
			break;
		case 'v':
			verbose = 1;
			break;
		case 'a':
			sleep_min = validateInt(optarg);
			break;	
		case 'b':
			sleep_max = validateInt(optarg);
			break;
		case 'l':
			loop_count = validateInt(optarg);
			break;
		case 's':
			step = validateInt(optarg);
			break;
					
		case 'r':
			enable_rt = 1;
			break;	
			
		case 'o':
			outfile = optarg;
			break;
					
			
        case ':':
            if(optopt == 'e' || optopt == 'E')
                break;
            //printArgumentMissing(optopt);
            break;
        case '?':
            break;
        default:
            abort();
       }
        
	}
	if (enable_rt) {
		struct sched_param param;
		int my_pid = getpid();
		int low_priority, high_priority;

		high_priority = sched_get_priority_max(SCHED_FIFO);
		low_priority = sched_get_priority_min(SCHED_FIFO);

		if (sched_getparam(my_pid, &param) != 0){
			perror("sched_getparam failed");
			exit(EXIT_FAILURE);
		}

		param.sched_priority = high_priority;
		if (sched_setscheduler(my_pid, SCHED_FIFO, &param) != SCHED_FIFO){
			perror("sched_setscheduler failed");
			exit(EXIT_FAILURE);
		}		
	}


}

long loop(int numberLoop, int intSleep)
{
	//initialize everything
	struct timespec 	startTime;		//store the realtime starting time
	struct timespec 	endTime;		//store the realtime ending time
	struct timespec		sleepTime;	
	long 				startNanoSec;	//convert and store the nanosec from startTime
	long  				endNanoSec;		//convert and store the nanosec from endTime	
	long				differenceTime; 		// to store the difference time
	long 				delay;				//delay? delay!
	long 				maxDelay = 0;						// store the maximum of delay
	

	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = intSleep;

	
	//start the loop
	int i;
	for(i = 0; i < numberLoop; ++i) {
	
		//Here we got the timemeasure 
		clock_gettime(CLOCK_MONOTONIC, &startTime);
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleepTime, NULL);
		clock_gettime(CLOCK_MONOTONIC, &endTime);
	
	
		//convert and store the measure
		startNanoSec = startTime.tv_nsec;
		endNanoSec = endTime.tv_nsec;
	
	
		//get the differnece between startTime and endTime
		differenceTime = endNanoSec - startNanoSec;
		delay = differenceTime - sleepTime.tv_nsec;
		
		
		// get the maximum of all differnceTime
		if(delay > maxDelay) {
			maxDelay = delay;
		}
		
		//store the datas in the valueArray
		valueArray[0 + (i * 3)] = startNanoSec;
		valueArray[1 + (i * 3)] = endNanoSec;
		valueArray[2 + (i * 3)] = differenceTime;
		
		
		if (verbose) printf("Start: %ld, \t End: %ld, \t Difference: %ld, \t Delay %ld\n", startNanoSec, endNanoSec, differenceTime, delay);
	}
	
	printf("The sleeping time of clock_nanosleep is %d nanosec. \n", sleepTime.tv_nsec);
	printf("The maximum of all delays is %ld nanosec \n", maxDelay);
	return maxDelay;
}


int main(int argc, char* argv[])
{
	FILE* fd = NULL;
	
	printf("Measure Resy-Grp4\n");
	parseOptions(argc, argv);
	
	int stepCount = (int) ceil(((double) (sleep_max - sleep_min)) / (double) step);
	int totalCount = stepCount * loop_count;	
	if (verbose) printf("stepCount %i, loopCount %i, totalCount %i, enablert %d\n", 
						stepCount, loop_count, totalCount, enable_rt); 
	
	//couse we have start- end and difference time we need 3 times more space	
	valueArray = (long*)malloc(sizeof(long) * 3 * totalCount);	
	
	if (outfile != NULL){
		fd = fopen(outfile, "w+");
	}
	
	for(int curSleep = sleep_min; curSleep < sleep_max; curSleep += step)  {
		long maxDelay = loop(loop_count, curSleep);
		if (fd != NULL){
			fprintf(fd, "%d\t%d\n", curSleep, maxDelay);
		}
	}
	
	if (fd != NULL){
		fclose(fd);
	}
	
	free(valueArray);
	
	return EXIT_SUCCESS;
	
}
	
