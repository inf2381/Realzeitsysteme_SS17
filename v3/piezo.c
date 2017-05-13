#include "piezo.h"
#include "gpio.h"

#include <unistd.h>


void piezoSetup() {
    GPIO_export(PIEZO_PIN);
    GPIO_setDirection(PIEZO_PIN, PIN_OUT);
}

void piezoSetdown() {
    GPIO_unexport(PIEZO_PIN);
}

void playTone() {
    GPIO_set(PIEZO_PIN, 1);
    sleep(1);
    GPIO_set(PIEZO_PIN, 0);
}
