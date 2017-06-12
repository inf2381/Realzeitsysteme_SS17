#ifndef GPIO_H
#define GPIO_H

#define PIN_IN "in"
#define PIN_OUT "out"

#define GPIO_HIGH 1
#define GPIO_LOW 0

extern int verbose;

void initPathCache();
void destroyPathCache();

char* GPIO_getPath(char* pin, const char* suffix);
int GPIO_read(char* pin);
void GPIO_set(char* pin, char value);

void GPIO_export(char* pin);
void GPIO_unexport(char* pin);
void GPIO_setDirection(char* pin, char* direction);

#endif




