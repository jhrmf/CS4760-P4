#include "sharedMem.h"

struct myTime updateClock(struct myTime virtual){     //function for updating the system clock's seconds and nanoseconds
    virtual.nanoseconds = virtual.nanoseconds + 40000;                       //increment nanoseconds by 40000 every call
    if(virtual.nanoseconds >= 1000000000){                          //if the nanoseconds exceeds or is equal to a second
        virtual.seconds++;                                                             //increment the seconds counter
        virtual.nanoseconds = virtual.nanoseconds - 1000000000;             //decrement the nanosecond count by a second
    }
    return virtual;                                           //return the new time stored in the virtual time structure
}

float getLaunchTime(int maxTimeBetweenNewProcsNS, int maxTimeBetweenNewProcsSecs){
    float launchTime;
    if(maxTimeBetweenNewProcsNS != 0){
        srand((unsigned)time(0));
        launchTime = (rand() % maxTimeBetweenNewProcsNS) / 1000000000;
    }
    if( maxTimeBetweenNewProcsSecs != 0){
        srand((unsigned)time(0));
        launchTime = launchTime + (rand() % maxTimeBetweenNewProcsSecs);
    }
    return launchTime;
}

static void myhandler(int s) {                                    //handler for the program to shut down at end of timer
    int errsave;
    errsave = errno;
    shmctl(shmget(66, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    shmctl(shmget(67, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    shmctl(shmget(65, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    shmctl(shmget(68, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    exit(0);                                                           //close program in its tracks after timer expires
    errno = errsave;
}
static int setupinterrupt(void) {                                                     /* set up myhandler for SIGPROF */
    struct sigaction act;
    act.sa_handler = myhandler;
    act.sa_flags = 0;
    return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}
static int setupitimer(int n) {                                             /* set ITIMER_PROF for n-second intervals */
    struct itimerval value;
    value.it_interval.tv_sec = n;                //set the timer to however many seconds user enters or default 1 second
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    return (setitimer(ITIMER_PROF, &value, NULL));
}

int main(int argc, char *argv[]) {

//    int opt = 0;

    int timeInSec = 5;                                                               //default timer is set to 5 seconds
    int maxChildren = 5;                                                       //default max child processes is set to 5
    int i = 0;
    int maxTimeBetweenNewProcsNS = 1000000000, maxTimeBetweenNewProcsSecs = 0;
    float launchItUp = getLaunchTime(maxTimeBetweenNewProcsNS, maxTimeBetweenNewProcsSecs);
    char fileName[100] = "logFile";                                            //default log file name is set to logFile
    virtual.nanoseconds = 0;                                                            //nanosecond counter is set to 0
    virtual.seconds = 0;                                                                    //second counter is set to 0

    setupinterrupt();                                                    //start the interrupt and timer for the program
    setupitimer(timeInSec);

    key_t secKey = 66;                                         //the key for the shared memory holding the seconds is 66
    key_t nanoKey = 67;                                    //the key for the shared memory holding the nanoseconds is 67

    int secID = shmget(secKey, 2048, 0666|IPC_CREAT);    //access the shared memory with file write and read permissions
    int nanoID = shmget(nanoKey, 2048, 0666|IPC_CREAT); //same as above, secID is for seconds, nanoID is for nanoseconds
    int childCount;                                      //this is for a loop below to hold the count of child processes
    pid_t childPid;                                            //of course we need one of these guys for child processes

   key_t dataKey = 68;
    int dataId = shmget(dataKey, 2048, 0666|IPC_CREAT);
   /* table = shmat(dataId, NULL, 0);
    table[0].CPUTime = 10;
    shmdt(table);
    struct BLOCK *test;
    test = shmat(shmget(dataKey, 2048, 0444), NULL, 0);
    printf("CPU Time is %d\n", test[0].CPUTime);
    printf("CPU Time for other is %d\n", test[1].CPUTime);
    shmctl(dataId, IPC_RMID, NULL);
*/
    do{
        tempTime = updateClock(tempTime);
        float checkTime = tempTime.seconds + ((float)tempTime.nanoseconds/1000000000);
        if(checkTime >= launchItUp) {
            printf("Launched at %f!\n", checkTime);
            tempTime.seconds = 0;
            tempTime.nanoseconds = 0;
            if (i < 19) {
                table = shmat(dataId, NULL, 0);
                table[i].CPUTime = 0;
                table[i].sysTime = 0;
                table[i].burstTime = 0;
                table[i].simPid = i + 100;
                table[i].priority = 0;
                shmdt(table);
                i++;
            }
        }

        virtual = updateClock(virtual);

    } while (virtual.seconds < 3 || childCount <= 100);   //this do while loop ends if seconds < 3 or 100 children exist

    shmctl(secID, IPC_RMID, NULL);                                                //delete the shared memory for seconds
    shmctl(nanoID, IPC_RMID, NULL);                                           //delete the shared memory for nanoseconds

    table = shmat(dataId, NULL, 0);
    for(i=18; i > -1; i--){
        printf("CPU Time: %d\n", table[i].CPUTime);
        printf("System Time: %d\n", table[i].sysTime);
        printf("Burst Time: %d\n", table[i].burstTime);
        printf("Simulated PID: %d\n", table[i].simPid);
        printf("Priority: %d\n", table[i].priority);
        printf("\n");
    }
    shmdt(table);

    shmctl(dataId, IPC_RMID, NULL);
    exit(0);                                                                    // end the process, cause its the parent

}