//
//  switch.c
//  
//
//  Created by Marc Bormeth on 29.03.17.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

const int PATHSIZE = 15;
const char* PATH = "/sys/class/gpio";

int readGPIO(int pin, int mode) {
    int value = -1;
    
    int sizeConcat = PATHSIZE + 1;
    
    if (pin >= 10)
        sizeConcat++;
    
    char concatPath = [sizeConcat];
    sprintf(concatPath, "%s%d", PATH, pin);
    
    if ((int gpio = open(concatPath, mode)) != -1){
        value = read(gpio);
        close(concatPath)
    }
    
    return value;
}


int main() {
    
    
    
    
    return EXIT_SUCCESS;
}
