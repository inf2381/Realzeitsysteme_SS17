#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#include "engine.h"


void sig_handler(int signo)
{
    if (signo == SIGINT){
        engineStop();
        engineSetdown();
        exit(EXIT_SUCCESS);
    }
}


void setUp() {

	engineSetup();

}


int main(int argc, char *argv[]) {

	setUp();

	if (signal(SIGINT, sig_handler) == SIG_ERR){
        	exit(EXIT_FAILURE);
    }

	while (true) {

		engineDrive(forward, forward);
		sleep(5);

	}

	return EXIT_SUCCESS;
}
