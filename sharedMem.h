#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
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

struct mesg_buffer {                                                                      //struct for the message queue
    long mesg_type;
    float timeQuantum;
    int pidToRun;
    char mesg_text[100];
} message;

struct BLOCK                                                         //struct for the process table required for project
{
    float CPUTime;
    float sysTime;
    float burstTime;
    int simPid;
    int job;
    int priority;
    int run;

};

struct myTime virtual;                                                             //struct for holding the virtual time
struct myTime tempTime;                                                          //struct for holding the temporary time
struct myTime local;                                                                //struct for the local time in userP
struct BLOCK *table;                                                 //struct for the process table required for project

const int getRandom(const int max, const int min){            //function for getting a random number between max and min
    int randomNumber = ((rand() % (max + 1 - min)) + min);
    return randomNumber;
}


#endif