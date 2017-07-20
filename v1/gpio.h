#ifndef GPIO_H
#define GPIO_H

#define PIN_IN "IN"
#define PIN_OUT "OUT"

extern int verbose;

char* GPIO_getPath(char* pin, const char* suffix);
int GPIO_read(char* pin);
void GPIO_set(char* pin, char value);

void GPIO_export(char* pin);
void GPIO_unexport(char* pin);
void GPIO_setDirection(char* pin, char* direction);

#endif



