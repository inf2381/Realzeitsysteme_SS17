#include "piezo.h"
#include "gpio.h"
#include "helper.h"

#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>


pthread_t enerver;
struct timespec sleeptime;
volatile bool shouldTerminate = false;

void piezoSetup() {
    GPIO_export(PIEZO_PIN);
    GPIO_setDirection(PIEZO_PIN, PIN_OUT);
}


void piezoSetdown() {
    GPIO_unexport(PIEZO_PIN);
}


void *reverseTone() {
    while (!shouldTerminate) {
        GPIO_set(PIEZO_PIN, 1);
        
        usleep(500000);  //0.5s
        GPIO_set(PIEZO_PIN, 0);
        usleep(250000);  //0.25s
    }
    GPIO_set(PIEZO_PIN, 0);
    pthread_exit(0);
}

void piezo_playReverse() {
    shouldTerminate = false;
    pthread_create(&enerver, NULL, reverseTone, NULL);
}


void piezo_stopReverse() {
    shouldTerminate = true;
}
                   

void playTone() {
    GPIO_set(PIEZO_PIN, 1);
    usleep(MICROSECONDS_PER_SECOND);
    GPIO_set(PIEZO_PIN, 0);
}
