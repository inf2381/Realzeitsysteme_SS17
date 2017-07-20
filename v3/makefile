### .........  MAKEFILE .........................####
###                                              ####
### .......generic Makefile .....................####
CC=gcc
CFLAGS=-O1 -Wall -std=gnu99 -I. -I./rfid

CPLUS = g++
CPLUS_FLAGS = -std=c++0x -O1

LDFLAGS=-L. -L./$(BCM_PATH)src -lrt  -lpthread -lbcm2835

STATIC_LIB = ar
ECHO = echo

BCM_PATH = rfid/bcm2835-1.50/
BCM_MAKECHAIN = autoscan; autoheader; automake --add-missing; aclocal; autoconf; ./configure

OBJFILES = engine.o gpio.o helper.o ultrasonic.o infrared.o logic.o piezo.o rfid.o rfid/bridging_header.o rfid/MFRC522.o killswitch.o
TARGET= rfid_wrapper engine stop

#Def Targets
.PHONY: all clean
all: $(TARGET)

clean:
	rm -f -r *.o *.a *.so $(TARGET) $(OBJFILES) 
	
#Concrete		
engine: main.o $(OBJFILES) 
rfid_wrapper: libbcm2835 rfid/bridging_header.o rfid/MFRC522.o
stop: stop.o $(OBJFILES) 

#Musterreglen
%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

%:%.o
	$(CPLUS) $(CPLUS_FLAGS) $^ $(LDFLAGS) -o $@

%.a:%.o
	$(STATIC_LIB) -rs $@ $^

%.o:%.cpp
	$(CPLUS) $(CPLUS_FLAGS) -o $@ -c $^

#Sub makefiles
libbcm2835 : $(BCM_PATH)src/libbcm2835.a
	#$(ECHO) looking into $(BCM_PATH) : $(MAKE) $(MFLAGS)
	#cd $(BCM_PATH); $(BCM_MAKECHAIN);  $(MAKE) $(MFLAGS)

$(BCM_PATH)src/libbcm2835.a : $(BCM_PATH)src/bcm2835.o	
	$(STATIC_LIB) -rs $@ $^

$(BCM_PATH)src/bcm2835.o : $(BCM_PATH)src/bcm2835.c
	$(CC) $(CFLAGS) -w -o $@ -c $^



force_look :
	true
