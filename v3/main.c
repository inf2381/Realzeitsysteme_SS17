#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>

#include "common.h"
#include "engine.h"
#include "ultrasonic.h"
#include "infrared.h"
#include "logic.h"
#include "piezo.h"
#include "rfid.h"
#include "helper.h"
#include "gpio.h"

// preparation for loadable kernel module
// glibc does not provide a header entry
#define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)
#define delete_module(name, flags) syscall(__NR_delete_module, name, flags)

pthread_t thread_us, thread_ir, thread_rfid, thread_exploit, thread_engine;
pthread_t* all_threads[] = {&thread_exploit, &thread_engine, &thread_us, &thread_ir, &thread_rfid, NULL};

pthread_rwlock_t ir_lock, us_lock, rfid_lock;
thread_args ir_args, us_args, rfid_args;
exploiterParams explParam;
volatile engineMode engineCtrl; //see common.h
volatile int shouldRun = 1;     //see common.h


int default_logicmode = track_path;
int default_degree = 90;

// Scheduling stuff
cpu_set_t cpuset_logic;     //see common.h
cpu_set_t cpuset_sensors;   //see common.h
cpu_set_t cpuset_engine;    //see common.h

/**
 * Generic function to load a kernel module
 * @param moduleName: Relative path to .ko file
 * @param params: LKM defined them with module_param()
 * @return: Ptr to loaded module
 */
void *loadKernelModule(const char *moduleName, const char *params) {
    int fd;
    if ((fd = open(moduleName, O_RDONLY)) < 0) {
        perror("Open .ko");
        exit(EXIT_FAILURE);
    }
    
    struct stat st;
    fstat(fd, &st);
    size_t image_size = st.st_size;
    void *image = malloc(image_size);
    enforceMalloc(image);  //see helper.h
    if ((read(fd, image, image_size)) < 0) {
        perror("Read .ko");
        exit(EXIT_FAILURE);
    }
    close(fd);
    
    if (init_module(image, image_size, params) != 0) {
        perror("Insmod");
        exit(EXIT_FAILURE);
    }
    
    return image;
}


void unloadKernelModule(void *img, char *name) {
    free(img);
    if (delete_module(name, O_NONBLOCK) != 0) {
        perror("Delete module");
        exit(EXIT_FAILURE);
    }
}

void setup() {
    CPU_ZERO(&cpuset_logic);
    CPU_ZERO(&cpuset_sensors);
    CPU_ZERO(&cpuset_engine);
    
    
    CPU_SET(CPU_LOGIC , &cpuset_logic);
    CPU_SET(CPU_SENSORS , &cpuset_sensors);
    CPU_SET(CPU_ENGINE , &cpuset_engine);
    
    initPathCache();
	engineSetup();
    ultrasonicSetup();
	infraredSetup();
    piezoSetup();
    rfidSetup();

	logic_setup(default_logicmode);
    engineCtrl = STAY;
    
    /*
    // LKM
    int pid = getpid();
    char *mod_params = (char*) malloc(10* sizeof(char));
    sprintf(mod_params, "pid=%d", pid);
    loadKernelModule("Kernel/killswitch.ko", mod_params);
    */
}

void shutdown(){
    logic_shutdown();
    
    shouldRun = 0;
	engineSetdown();
	ultrasonicSetdown();
	infraredSetdown();
    piezoSetdown();
    rfidSetdown();
    destroyPathCache();
    
    pthread_t* ptr = all_threads[0];
    /*
    printf("%p, t %p, e %p \n", ptr, &thread_us, &thread_engine);
    while (*ptr) {
        printf("%p, t %p, %d\n", &ptr, ptr, *ptr == 0);
        pthread_cancel(*ptr);
        ptr++;
    }
    */
    
    
    // unloadKernelModule("killswitch");
    
    printf("cancel");
    ptr = all_threads[0];
    while (*ptr) { 
        printf("%p, t %p, %d\n", &ptr, ptr, *ptr == 0);
        pthread_join(*ptr, NULL);
        ptr++;
    }
   
    pthread_rwlock_destroy(&ir_lock);
    pthread_rwlock_destroy(&us_lock);
    pthread_rwlock_destroy(&rfid_lock);
}


void initArgsGeneric(thread_args* args, pthread_rwlock_t* lock){
	if (pthread_rwlock_init(lock, NULL)) {
		perror("genric_lock_init");
        exit(EXIT_FAILURE);
    }

	args->lock = lock;
	args->timestamp = 0;
	args->data = NULL;
} 

void sig_handler(int signo)
{
    if (signo == SIGINT){
        shutdown();
        //pthread_exit(0);
        exit(EXIT_SUCCESS);
    }
}

void readCommandLine(int argc, char *argv[]){
     while (42) {
        static struct option long_options[] = {
            {"mode", required_argument, 0, 'm'},
            {"degree", required_argument, 0, 'd'},
            {NULL, 0, 0, 0}
        };
        int option_index = 0;
        int option = getopt_long_only(argc, argv, "m:d:", long_options, &option_index);

        if (option == -1)
            break;

        switch(option){
            case 'm':
                if (strcmp(optarg, "rfid") == 0) {
                    default_logicmode = test_rfid;

                } else if (strcmp(optarg, "ir") == 0) {
                    default_logicmode = test_ir;

                } else if (strcmp(optarg, "us") == 0) {
                    default_logicmode = test_us;

                } else if (strcmp(optarg, "piezo") == 0) {
                    default_logicmode = test_piezo;

                } else if (strcmp(optarg, "engine") == 0) {
                    default_logicmode = test_engine;

                } else if (strcmp(optarg, "path") == 0) {
                    default_logicmode = track_path;

                } else if (strcmp(optarg, "search") == 0) {
                    default_logicmode = track_rfid_search;
                    
                }  else if (strcmp(optarg, "turn") == 0) {
                    default_logicmode = test_turn;
                }
                break;
     
            case 'd': 
                default_degree = validateInt(optarg);
                break;
            case ':':
                if(optopt == 'e' || optopt == 'E')
                    break;
                printArgumentMissing(optopt);
                break;
            case '?':
                break;
             default:
                break;
        }
    }
}


int main(int argc, char *argv[]) {
    int exploitThreadRet;
    
	printf("RESY ROBOT - TEAM 4\n");  
	if (signal(SIGINT, sig_handler) == SIG_ERR){
        exit(EXIT_FAILURE);
    }

    readCommandLine(argc, argv);
    setup();

    //preparing structs
	initArgsGeneric(&ir_args, &ir_lock);
	initArgsGeneric(&us_args, &us_lock);
	initArgsGeneric(&rfid_args, &rfid_lock);
    
    explParam.ir = &ir_args;
    explParam.us = &us_args;
    explParam.rfid = &rfid_args;
    
    //starting threads
    pthread_create(&thread_us, NULL, measureDistance, (void*) &us_args);
	pthread_create(&thread_ir, NULL, infrared_read, (void*) &ir_args);
    pthread_create(&thread_rfid, NULL, detectRFID, (void*) &rfid_args);
    pthread_create(&thread_exploit, NULL, exploitMeasurements, (void*) &explParam);
    pthread_create(&thread_engine, NULL, engineController, NULL);
    
    //wait for exploiting thread to finish
    pthread_join(&thread_exploit, (void**) &exploitThreadRet);
    if (exploitThreadRet) {  //exploitThreadRet contains the current logicMode
        // TODO: Save status, reboot
    }

	shutdown();
	return EXIT_SUCCESS;
}
