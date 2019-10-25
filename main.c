#include "sharedMem.h"
#include "queue.h"

/*The code for the bitMap was provided by Bhatia, who provided it to his students. If this is viewed negatively,
                                 or in the sense of plagiarism, it is not intended since it was professor provided code.
                                                                          I can switch to using a bit array if need be*/

int openBit(unsigned char * bitMap, int lastPid){
    lastPid++;                                                                          //increment the pid index passed
    if(lastPid > 17){                                                                           //if its greater than 17
        lastPid = 0;                                                                                  //set the pid to 0
    }
    int tempPid = lastPid;                                                                            //get a temp value
    while(1){
        if((bitMap[lastPid/8] & (1 << (lastPid % 8 ))) == 0){                     //check if the bit has been set or not
            /*bit has not been set yet*/
            return lastPid;                                                           //if not than return the pid index
        }

        lastPid++;                                                                       //increment the pid index again
        if(lastPid > 17){                                                       //again, set to 0 if its greater than 17
            lastPid = 0;
        }
        if( lastPid == tempPid){                      //if the pid and the temp are equal, than there are no free spaces
            return -1;
        }

    }
}


//Just flips the bit to 1  in the given bitmap
//at the given lastPid.
void setBit(unsigned char * bitMap, int lastPid){                                              //mark the bit in the map
    bitMap[lastPid/8] |= (1<<(lastPid%8));
}

//Just flips the bit to 0 in the given bitmap and given LastPid
void resetBit(unsigned char * bitMap, int finPid){
    bitMap[finPid/8] &= ~(1 << (finPid % 8));                                                //unmark the bit in the map
}

struct myTime updateTimeAfterProcess(struct myTime virtual, float quantum){
    while(quantum != 0){                                                               //as long as the quantum is not 0
        if(quantum >= 1){                                                             //if the quantum is greater than 1
            virtual.seconds++;                                  //add a second to the time and deduce 1 from time to add
            quantum--;
        }
        else{                                                 //otherwise, add the remaining decimals to the nanoseconds
            virtual.nanoseconds += (quantum*1000000000);
            break;
        }
    }
    return virtual;                                                                                //return updated time
}

struct myTime updateClock(struct myTime virtual){     //function for updating the system clock's seconds and nanoseconds
    virtual.nanoseconds = virtual.nanoseconds + 80000;                       //increment nanoseconds by 40000 every call
    if(virtual.nanoseconds >= 1000000000){                          //if the nanoseconds exceeds or is equal to a second
        virtual.seconds++;                                                             //increment the seconds counter
        virtual.nanoseconds = virtual.nanoseconds - 1000000000;             //decrement the nanosecond count by a second
    }
    return virtual;                                           //return the new time stored in the virtual time structure
}

float getLaunchTime(int maxTimeBetweenNewProcsNS, int maxTimeBetweenNewProcsSecs){    //function for getting launch time
    float launchTime;
    if(maxTimeBetweenNewProcsNS != 0){                                                       //as long as value is not 0
        launchTime = getRandom(maxTimeBetweenNewProcsNS, 0) / 1000000000;            //get a random time for nanoseconds
    }
    if( maxTimeBetweenNewProcsSecs != 0){                                                    //again, as long as not a 0
        launchTime = launchTime + getRandom(maxTimeBetweenNewProcsSecs, 0);              //get a random time for seconds
    }
    return launchTime;
}

static void myhandler(int s) {                                    //handler for the program to shut down at end of timer
    int errsave;
    key_t key = ftok("oss", 70);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    errsave = errno;
    shmctl(shmget(66, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    shmctl(shmget(67, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    shmctl(shmget(65, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    shmctl(shmget(68, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    msgctl(msgid, IPC_RMID, NULL);
    system("killall userP");
    system("killall oss");
    printf("Terminated");
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

    char unsigned  bitMap[3];                                                                                   //bitmap
    memset(bitMap, '\0', sizeof(bitMap));                                                            //initialize bitmap
    int lastPid = -1;                                                                                //set lastPid to -1

    struct Queue* zero = createQueue();                                          //three queues for different priorities
    struct Queue* one = createQueue();
    struct Queue* two = createQueue();

    int timeInSec = 60;                                                              //default timer is set to 5 seconds
    int maxChildren = 5;                                                       //default max child processes is set to 5
    int i = 0;
    int maxTimeBetweenNewProcsNS = 999999999, maxTimeBetweenNewProcsSecs = 2;                       //constant max times
    int opt = 0;
    char fileName[100] = "logFile";                                            //default log file name is set to logFile
    virtual.nanoseconds = 0;                                                            //nanosecond counter is set to 0
    virtual.seconds = 0;                                                                    //second counter is set to 0

    while ((opt = getopt(argc, argv,"hs:l:t:")) != -1) {                                                //GET OPT WOOOOO
        switch (opt) {                                          //start a switch statement for the commandline arguments
            case 'h' :                                                                              //print help message
                printf("Welcome to OSS Scheduler\n");
                printf("Invocation: ./oss [-h] [-t timeForExecution] \n");
                printf("h : Display a help message to the user and exit \n");
                printf("t : Limit the program to n amount of seconds for execution (Default is 60 Seconds)\n");
                printf("Note: If n seconds have passed with -t , a logFile will not be created with termination\n");
                exit(0);
                break;
            case 't' :                                                     //set the time entire program can execute for
                timeInSec = atoi(optarg);
                break;
            default:                                                              //user inputted something unrecognized
                perror("Command line argument not recognized.");
                exit(EXIT_FAILURE);
        }

    }


    setupinterrupt();                                                    //start the interrupt and timer for the program
    setupitimer(timeInSec);

    int childCount;                                      //this is for a loop below to hold the count of child processes
    pid_t childPid;                                            //of course we need one of these guys for child processes

    key_t dataKey = 68;
    int dataId = shmget(dataKey, 2048, 0666|IPC_CREAT);


    float pLaunch = (float)(getRandom(maxTimeBetweenNewProcsSecs, 0) +             //get initial launch time for process
            (float)((float)getRandom(maxTimeBetweenNewProcsNS, 0)/1000000000));
    int msgid;
    key_t secKey = 66;                                         //the key for the shared memory holding the seconds is 66
    key_t nanoKey = 67;                                    //the key for the shared memory holding the nanoseconds is 67
    key_t killedKey = 30;                                  //the key for the shared memory for checking if process ended

    int secID = shmget(secKey, 2048, 0666|IPC_CREAT);    //access the shared memory with file write and read permissions
    int nanoID = shmget(nanoKey, 2048, 0666|IPC_CREAT); //same as above, secID is for seconds, nanoID is for nanoseconds
    int killID = shmget(killedKey, 2048, 0666|IPC_CREAT);                 //the id for the ended processes shared memory
    remove("logFile");                                //remove whatever the default log file name just in case it exists
    FILE *logptr = fopen(fileName, "w");                                               //initialize file ptr for writing
    while(virtual.seconds < 200 && i <= 100){                //this while loop ends if seconds < 3 or 100 children exist
        tempTime = updateClock(tempTime);           //update a temporary clock for checking if process run time is ready
        float checkTime = (float)tempTime.seconds + ((float)tempTime.nanoseconds/1000000000);    //update the check time
        if(checkTime >= pLaunch) {                    //check if the time is equivalent to the launch time for a process
            if(i < 17){                                        //as long as there aren't 18 running processes (i = 0-17)
                lastPid = openBit(bitMap, lastPid);                      //check if there is an open index in the bitmap
                if(lastPid != -1){                                                            //if there is an open spot
                    setBit(bitMap, lastPid);                                           //set the location in map as used
                    table = shmat(dataId, NULL, 0);                                     //get the table in shared memory
                    table[lastPid].CPUTime = 0.0;                            //intiailize all to zero other than sysTime
                    table[lastPid].sysTime = (float)((float)virtual.seconds +
                            (float)((float)virtual.nanoseconds/1000000000));
                    table[lastPid].burstTime = 0.0;
                    table[lastPid].priority = 0;
                    i++;                                                                //incrememnt the process counter
                    childPid = fork();                                                                       //FORK OFF!
                    if(childPid == 0){                                               //the child should to the following
                        table[lastPid].simPid = getpid();                                     //set the PID in the table
                        shmdt(table);                                                        //detach from shared memory
                        execl("./userP", NULL);                                                     //run the file userP

                    }
                    table = shmat(dataId, NULL, 0);                                  //parent should attach to the table

                    while(table[lastPid].simPid == 0){                               //wait until PID is filled by child
                        //do nothing
                    }

                    char msg[100] = "Generating process with PID ";        //from here to below we are writing a message
                    char temp[100] = "\0";                                                            //for the log file
                    sprintf(temp, "%d", table[lastPid].simPid);
                    strcat(msg, temp);
                    strcpy(temp, "\0");
                    strcat(msg, " and putting it in queue at time ");
                    sprintf(temp, "%f", table[lastPid].sysTime);
                    strcat(msg, temp);
                    strcat(msg, "\n");
                    fputs(msg, logptr);                                                       //end of message and write
                    enQueue(zero, lastPid);                                                    //enqueue the process pid
                    shmdt(table);                                                         //detach from the memory table
                    pLaunch = checkTime + (float)(getRandom(maxTimeBetweenNewProcsSecs, 0) +
                            (float)(getRandom(maxTimeBetweenNewProcsNS, 0)/1000000000));           //get new launch time


                }
                else{                                                              //if there was no space in the bitmap
                    printf("No available space\n");
                    pLaunch += checkTime;                                                        //get a new launch time
                }

            }
            virtual = updateClock(virtual);                                                      //update the clock time
            tempTime.seconds = 0;                                               //reset the temporary clock for checking
            tempTime.nanoseconds = 0;
        }
        else if(zero->front != NULL){                                        //if the 0 priority queue has a value in it
            int pid = zero->front->key;                                                                    //get the pid
            deQueue(zero);                                                                           //dequeue the value
            i--;                                                                            //negate the process counter
            key_t key = ftok("oss", 70);
            msgid = msgget(key, 0666 | IPC_CREAT);
            message.mesg_type = 1;
            strcpy(message.mesg_text, "PROCESS ENTERED SYSTEM");
            message.timeQuantum = 1.2;                                                 //quantum is a constant value 1.2
            message.pidToRun = pid;
            msgsnd(msgid, &message, sizeof(message), 0);     //send a message the process waiting with valid information

            table = shmat(dataId, NULL, 0);                                              //attach to the table in memory
            while (table[pid].run == 0) {                                                  //while the process hasnt run
                virtual.nanoseconds++;                                         //increment the time for a small overhead
                if (virtual.nanoseconds >= 1000000000) {
                    virtual.nanoseconds -= 1000000000;
                    virtual.seconds++;
                }
                table = shmat(dataId, NULL, 0);
            }

            char *killStr = (char *) shmat(killID, (void *) 0, 0);   //ptr to the shared memory location for process end
            if(strcmp(killStr, "Killed") != 0){                                           //if the process has not ended
                enQueue(zero, pid);                                                                         //requeue it
            }
            else{                                               //otherwise reset the bit it corresponds to, and mark it
                resetBit(bitMap, pid);
                strcpy(killStr, "Alive");
            }
            shmdt(killStr);                                                     //detach from the shared memory location



            char msg[100] = "Dispatching process with PID ";                      //from here until below we are writing
            char temp[100] = "\0";                                                          // a message to the log file
            sprintf(temp, "%d", table[pid].simPid);
            strcat(msg, temp);
            strcpy(temp, "\0");
            strcat(msg, " and from queue 0 at time ");
            sprintf(temp, "%f", table[pid].burstTime);
            strcat(msg, temp);
            strcat(msg, "\n");
            strcat(msg, "Total time for dispatch was ");
            strcpy(temp, "\0");
            sprintf(temp, "%f", table[pid].CPUTime);
            shmdt(table);
            strcat(msg, temp);
            strcat(msg, "\n");
            fputs(msg, logptr);                                                //end of message construction and writing
            virtual = updateTimeAfterProcess(virtual, message.timeQuantum);         //update the time with the work time
            virtual = updateClock(virtual);                                                //incremement the clock again

        }
        else if(one->front != NULL){            //if the priority one queue is not empty, doe the same as above for zero
            int pid = one->front->key;
            deQueue(one);
            i--;
            key_t key = ftok("oss", 70);
            msgid = msgget(key, 0666 | IPC_CREAT);
            message.mesg_type = 1;
            strcpy(message.mesg_text, "PROCESS ENTERED SYSTEM");
            message.timeQuantum = 1.2;
            message.pidToRun = pid;
            msgsnd(msgid, &message, sizeof(message), 0);

            table = shmat(dataId, NULL, 0);
            while (table[pid].run == 0) {
                virtual.nanoseconds++;
                if (virtual.nanoseconds >= 1000000000) {
                    virtual.nanoseconds -= 1000000000;
                    virtual.seconds++;
                }
                table = shmat(dataId, NULL, 0);
            }

            char *killStr = (char *) shmat(killID, (void *) 0, 0);
            if(strcmp(killStr, "Killed") != 0){
                enQueue(one, pid);
            }
            else{
                resetBit(bitMap, pid);
                strcpy(killStr, "Alive");
            }
            shmdt(killStr);                                                     //detach from the shared memory location

            char msg[100] = "Dispatching process with PID ";
            char temp[100] = "\0";
            sprintf(temp, "%d", table[pid].simPid);
            strcat(msg, temp);
            strcpy(temp, "\0");
            strcat(msg, " and from queue 1 at time ");
            sprintf(temp, "%f", table[pid].burstTime);
            strcat(msg, temp);
            strcat(msg, "\n");
            strcat(msg, "Total time for dispatch was ");
            strcpy(temp, "\0");
            sprintf(temp, "%f", table[pid].CPUTime);
            shmdt(table);
            strcat(msg, temp);
            strcat(msg, "\n");
            fputs(msg, logptr);
            virtual = updateTimeAfterProcess(virtual, message.timeQuantum);
            virtual = updateClock(virtual);

        }
        else if(two->front != NULL){            //if the priority two queue is not empty, doe the same as above for zero
            int pid = two->front->key;
            deQueue(two);
            i--;
            key_t key = ftok("oss", 70);
            msgid = msgget(key, 0666 | IPC_CREAT);
            message.mesg_type = 1;
            strcpy(message.mesg_text, "PROCESS ENTERED SYSTEM");
            message.timeQuantum = 1.2;
            message.pidToRun = pid;
            msgsnd(msgid, &message, sizeof(message), 0);

            table = shmat(dataId, NULL, 0);
            while (table[pid].run == 0) {
                virtual.nanoseconds++;
                if (virtual.nanoseconds >= 1000000000) {
                    virtual.nanoseconds -= 1000000000;
                    virtual.seconds++;
                }
                table = shmat(dataId, NULL, 0);
            }

            char *killStr = (char *) shmat(killID, (void *) 0, 0);
            if(strcmp(killStr, "Killed") != 0){
                enQueue(two, pid);
            }
            else{
                resetBit(bitMap, pid);
                strcpy(killStr, "Alive");
            }
            shmdt(killStr);

            char msg[100] = "Dispatching process with PID ";
            char temp[100] = "\0";
            sprintf(temp, "%d", table[pid].simPid);
            strcat(msg, temp);
            strcpy(temp, "\0");
            strcat(msg, " and from queue 2 at time ");
            sprintf(temp, "%f", table[pid].burstTime);
            strcat(msg, temp);
            strcat(msg, "\n");
            strcat(msg, "Total time for dispatch was ");
            strcpy(temp, "\0");
            sprintf(temp, "%f", table[pid].CPUTime);
            shmdt(table);
            strcat(msg, temp);
            strcat(msg, "\n");
            fputs(msg, logptr);
            virtual = updateTimeAfterProcess(virtual, message.timeQuantum);
            virtual = updateClock(virtual);

        }
        else{                                                                         //otherwise, incremement the clock
            virtual = updateClock(virtual);
        }
        if(zero->front != NULL){                                      //when the queue 0 is not empty, reorganize queues

            int held = zero->front->key;
            struct Queue* tempZero = createQueue();                   //create new temporary queues for one two and zero
            struct Queue* tempOne = createQueue();
            struct Queue* tempTwo = createQueue();
            table = shmat(dataId, NULL, 0);                                       //attach to the table in shared memory
            float timeZero = 0, timeOne = 0, timeTwo = 0, avgZero, avgOne, avgTwo;
            int count = 1;
            while(zero->front != NULL){                      //while zero is not empty, move it into the temproray queue
                int pid = zero->front->key;                                //all the while, get the times of the process
                timeZero = timeZero + (table[pid].burstTime - table[pid].sysTime);
                count++;
                enQueue(tempZero, pid);
                deQueue(zero);
            }
            avgZero = timeZero / count;                            //calculate the average time based on the times above
            count = 1;
            while(one->front != NULL){                    //do the same here as we did above for zero, but for queue one
                int pid = one->front->key;
                timeOne += (table[pid].burstTime - table[pid].sysTime);
                count++;
                enQueue(tempOne, pid);
                deQueue(one);
            }
            avgOne = timeOne / count;
            count = 1;
            while(two->front != NULL){                    //do the same here as we did above for zero, but for queue two
                int pid = two->front->key;
                timeTwo += (table[pid].burstTime - table[pid].sysTime);
                count++;
                enQueue(tempTwo, pid);
                deQueue(two);
            }
            avgTwo = timeTwo / count;

            while(tempZero->front != NULL){                                 //while the temporary zero queue is not null
                int pid = tempZero->front->key;
                if(table[pid].CPUTime > (10*avgOne)){  //if the process time is greater than 9 times the avg of queue one
                    if(table[pid].CPUTime > (15*avgTwo)){       //if the process time is < 10 times the avg of queue two
                        char msg[100] = "Moving process with PID ";              //move the process into its new queue 2
                        char temp[100] = "\0";                                    //also write a message to the log file
                        sprintf(temp, "%d", table[pid].simPid);
                        strcat(msg, temp);
                        strcat(msg, " to queue 2\n");
                        fputs(msg, logptr);
                        deQueue(tempZero);
                        enQueue(two, pid);
                    }
                    else{
                        char msg[100] = "Moving process with PID ";              //move the process into its new queue 1
                        char temp[100] = "\0";                                    //also write a message to the log file
                        sprintf(temp, "%d", table[pid].simPid);
                        strcat(msg, temp);
                        strcat(msg, " to queue 1\n");
                        fputs(msg, logptr);
                        deQueue(tempZero);
                        enQueue(one, pid);
                    }
                }
                else{                                                               //otherwise, store back into queue 0
                    deQueue(tempZero);
                    enQueue(zero, pid);
                }
            }
            shmdt(table);                                                       //detach from the table in shared memory

        }

        char temp1[10], temp2[11];                                                   //two temp strings for holding time
        sprintf(temp1, "%d", virtual.seconds);                                           //store the seconds in a string
        sprintf(temp2, "%d", virtual.nanoseconds);                                   //store the nanoseconds in a string
        char *secStr = (char *) shmat(secID, (void *) 0, 0);             //ptr to the shared memory location for seconds
        strcpy(secStr, temp1);                                          //copy the seconds to the shared memory location
        shmdt(secStr);                                                          //detach from the shared memory location
        char *nanoStr = (char *) shmat(nanoID, (void *) 0, 0);       //ptr to the shared memory location for nanoseconds
        strcpy(nanoStr, temp2);                                     //copy the nanoseconds to the shared memory location
        shmdt(nanoStr);                                                         //detach from the shared memory location

    }
    shmctl(secID, IPC_RMID, NULL);                                                //delete the shared memory for seconds
    shmctl(nanoID, IPC_RMID, NULL);                                           //delete the shared memory for nanoseconds
    shmctl(killID, IPC_RMID, NULL);

    msgctl(msgid, IPC_RMID, NULL);
    fclose(logptr);
    printf("End\n");

    shmctl(dataId, IPC_RMID, NULL);
    exit(0);                                                                    // end the process, cause its the parent

}