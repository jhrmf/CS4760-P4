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

struct myTime{                                                                             //struct for our virtual time
    int seconds;
    int nanoseconds;
};

struct mesg_buffer {
    long mesg_type;
    char mesg_text[100];
} message;

struct BLOCK
{
    int CPUTime;
    int sysTime;
    int burstTime;
    int simPid;
    int priority;
    int job;
    int run;

};

struct myTime virtual;                                                             //struct for holding the virtual time
struct myTime tempTime;
struct BLOCK *table;
#endif