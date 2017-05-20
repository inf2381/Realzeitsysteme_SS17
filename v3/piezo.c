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
        
        sleepAbsolute(500000, &sleeptime); //0,5ms
        GPIO_set(PIEZO_PIN, 0);
        sleepAbsolute(250000, &sleeptime); //0,25ms
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
    sleepAbsolute(1 * NANOSECONDS_PER_SECOND, &sleeptime);
    GPIO_set(PIEZO_PIN, 0);
}
