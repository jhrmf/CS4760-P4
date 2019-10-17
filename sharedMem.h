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
#include <sys/shm.h>
#include <semaphore.h>
#include <pthread.h>

/*holy cow look at all these headers!*/

struct myTime{                                                                             //struct for our virtual time
    int seconds;
    int nanoseconds;
};

struct BLOCK
{
    struct BLOCK *link;  /* forward link for linked list */
    int CPUTime;
    int sysTime;
    int burstTime;
    int simPid;
    int priority;

};

struct myTime virtual;                                                             //struct for holding the virtual time
struct myTime tempTime;
struct BLOCK *table;
#endif