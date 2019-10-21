#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

/*holy cow look at all these headers!*/

#define ALPHA 3
#define BETA 2

struct myTime{                                                                             //struct for our virtual time
    int seconds;
    int nanoseconds;
};

struct mesg_buffer {
    long mesg_type;
    float timeQuantum;
    char mesg_text[100];
} message;

struct BLOCK
{
    float CPUTime;
    float sysTime;
    float burstTime;
    int simPid;
    int job;
    int run;

};

struct myTime virtual;                                                             //struct for holding the virtual time
struct myTime tempTime;
struct BLOCK *table;

const int getRandom(const int max, const int min){
    int randomNumber = ((rand() % (max + 1 - min)) + min);
    return randomNumber;
}
int getSeconds(){                               //function for retrieving the second in our time stored in shared memory
    key_t key = 66;
    int secID = shmget(key, 2048, 0444);
    char *tempTime = (char*) shmat(secID, (void*)0, 0);
    int seconds = atoi(tempTime);
    shmdt(tempTime);
    return seconds;
}
float getNano(){                            //function for retrieving the nanosecond our in time stored in shared memory
    key_t key = 67;
    int nanoID = shmget(key, 2048, 0444);
    char *tempTime = (char*) shmat(nanoID, (void*)0, 0);
    float nano = (float)(atoi(tempTime)) / 1000000000;
    shmdt(tempTime);
    return nano;
}

#endif