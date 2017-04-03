#include <stdio.h>
#include <stdlib.h>
#include <time.h>





int main(int args, char* argv[])
{
	
	//initialize everything
	struct timespec 	startTime;		//store the realtime starting time
	struct timespec 	endTime;		//store the realtime ending time
	long 				startNanoSec;	//convert and store the nanosec from startTime
	long  				endNanoSec;		//convert and store the nanosec from endTime
	
	const int			sleepingTime = 10000; // 10k nanosec = 1/100.000 sec
	long				differenceTime; 
	int 				loop = 100;
	char* p;
	
	
	
	if(argv[1] != NULL)
	{
		loop = strtol(argv[1], &p, 10);			//convert argv[0] to int
		
		if(*p != '\0')							//check if isn't possible
		{ 
			loop = 100;
			printf("Invalid input to set loops. Use default 100! \n");
		}
		 
		else 									// if its possible use it
		 
		{
			printf("We start the mesurement with %s loops \n", argv[1]);
			
		}
	}
	
	int i;
	for( i = 0; i < loop; ++i)
	{
	
		//Here we got the timemeasure 
		clock_gettime(CLOCK_REALTIME, &startTime);
		clock_nanosleep(CLOCK_REALTIME, sleepingTime ,NULL , NULL);
		clock_gettime(CLOCK_REALTIME, &endTime);
	
	
	
		//convert and store the measure
		startNanoSec 	= 	startTime.tv_nsec;
		endNanoSec		=  	endTime.tv_nsec;
	
	
	
		//get the differnece between startTime and endTime
		differenceTime = endNanoSec - startNanoSec;
	
		printf("Start: %ld \n", startNanoSec);
		printf("End: %ld \n", endNanoSec);
		printf("Difference: %ld \n", differenceTime);
	
	}
	
	return 0;
	
}
	
