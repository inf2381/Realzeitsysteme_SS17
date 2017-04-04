#include <stdio.h>
#include <stdlib.h>
#include <time.h>





int main(int args, char* argv[])
{
	
	//initialize everything
	struct timespec 	startTime;		//store the realtime starting time
	struct timespec 	endTime;		//store the realtime ending time
	struct timespec		sleepTime;	
	long 				startNanoSec;	//convert and store the nanosec from startTime
	long  				endNanoSec;		//convert and store the nanosec from endTime	
	long				differenceTime; 		// to store the difference time
	long 				delay;				//delay? delay!
	int 				loop = 100;			// how much loops should be done
	char* p;							// needed to convert argv to int to set loop times
	long maxDelay = 0;						// store the maximum of delay
	
	long *valueArray;							//store all values in a array
	
	
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = 1000000;
	
	if(argv[1] != NULL)
	{
		loop = strtol(argv[1], &p, 10);			//convert argv[0] to int
		
		if(*p != '\0')							//check if isn't possible
		{ 
			loop = 100;
			printf("Invalid input to set loops. Use default 100 loops! \n");
		}
		 
		else 									// if its possible use it
		 
		{
			printf("We start the mesurement with %s loops \n", argv[1]);
			
		}
	} else {
		printf("We start the mesurement with default 100 loops. \n");
	}
	
	valueArray = (long*)malloc(loop * sizeof(long) * 3);		//couse we have start- end and difference time we need 3 times more space
	
	
	
	
	printf("The sleeping time of clock_nanosleep is %d nanosec. \n", sleepTime.tv_nsec);
	
	
	//start the loop
	int i;
	for(i = 0; i < loop; ++i)
	{
	
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
		if(delay > maxDelay)
		{
			maxDelay = delay;
		}
		
		//store the datas in the valueArray
		valueArray[0 + (i * 3)] = startNanoSec;
		valueArray[1 + (i * 3)] = endNanoSec;
		valueArray[2 + (i * 3)] = differenceTime;
		
		
		printf("Start: %ld, \t End: %ld, \t Difference: %ld, \t Delay %ld\n", startNanoSec, endNanoSec, differenceTime, delay);
		
	
	}
	
	printf("The maximum of all delays is %ld nanosec \n", maxDelay);	
	free(valueArray);
	
	return 0;
	
}
	
