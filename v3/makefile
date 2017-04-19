### .........  MAKEFILE .........................####
###                                              ####
### .......generic Makefile .....................####
CC=gcc
CFLAGS=-Wall -g -std=gnu99 
LDFLAGS=-lpthread

TARGET=engine

#Def Targets
.PHONY: all clean
all: $(TARGET)

clean:
	rm -f -r *.o *.a *.so $(TARGET)
	
#Concrete		
engine: main.o engine.o gpio.o helper.o

#Musterreglen
%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

%:%.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	
