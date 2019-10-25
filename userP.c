#include "sharedMem.h"

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

struct myTime updateTimeAfterDifference(struct myTime virtual, float difference){           //function for updating time
    while(difference != 0){                                                    //for the distance calculation used below
        if(difference >= 1){
            virtual.seconds++;
            difference--;
        }
        else{
            virtual.nanoseconds += difference;
            break;
        }
    }
    return virtual;
}

struct myTime updateLocal(struct myTime temp){                                    //function for updating the local time
    temp.nanoseconds = temp.nanoseconds + 30000;
    if(temp.nanoseconds >= 1000000000){
        temp.seconds++;
        temp.nanoseconds = temp.nanoseconds - 1000000000;
    }
    return temp;
}

int work(struct BLOCK *table, float quantum){                          //function for handling the "work" a process does
    int end = 1;
    srand(time(0));
    local.seconds = 0;
    local.nanoseconds = 0;
    int job = getRandom(100, 0);            //get a random number between 0 and 100 to decide what work the process does
    if(job < 5){                                          //give the process a 5% chance of ending instead of doing work
        job = 0;
        end = 2;
    }
    else if(job < 30){                           //give the process a 25% chance for job 2 which would result in a block
        job = 2;
    }
    else if(job < 65){                                     //give the process a 35% chance for jobs 1 and 3 respectively
        job = 1;
    }
    else{
        job = 3;
    }


    printf("Executing job: %d \n", job);                                   //print out to console the job being executed
    if(job == 0){                                                             //job 0 adds 5000 nanoseconds for overhead
        local.nanoseconds += 5000;
        printf("Process terminating at %f\n", (float)((float)getSeconds() + getNano()));
    }
    if(job == 1){                                                                             //run for the time quantum
        while(1){
            local = updateLocal(local);
            if((float)((float)local.seconds + (float)((float)local.nanoseconds/1000000000)) >= quantum){
                break;
            }
        }
    }
    if(job == 2){                                                              //run for a certain random amount of time
        int r = getRandom(5, 0);
        int s = getRandom(1000, 0);
        float difference = ((float)r + ((float)s / 10000));
        int currentSecond = local.seconds;
        float currentNano = local.nanoseconds/1000000000;
        while(1){
            if(((float)((float)local.seconds + local.nanoseconds) -
                    (float)((float)currentSecond + currentNano)) >= difference){
                updateTimeAfterDifference(local, difference);
                                                                            //block would occur here but not implemented
                break;
            }
            local = updateLocal(local);                                                              //update local time
        }
    }
    if(job == 3){                                                          //run for certain percent of the time quantum
        int p = getRandom(100, 1);
        float localCheck;
        while(1){
            localCheck = (float)((float)local.seconds + (float)(local.nanoseconds/1000000000));
            if(localCheck >= (float)((float)(p*quantum)/100)){
                break;
            }
            local = updateLocal(local);                                                              //update local time
        }
    }

    message.timeQuantum = (float)((float)local.seconds +
            (float)((float)local.nanoseconds/1000000000));                    //update the time quantum in message queue
    table[message.pidToRun].CPUTime = (float)((float)local.seconds +
            (float)((float)local.nanoseconds/1000000000));                                     //update the process time
    table[message.pidToRun].run = 1;                                                          //set table run value to 1
    return end;
}

int main(int argc, char *argv[]){

    int msgid;                                                                                //id for the message queue

    key_t killedKey = 30;                                             //key for the shared memory that holds process end
    int killID = shmget(killedKey, 2048, 0666|IPC_CREAT);                                        //get the shared memory

    key_t key = ftok("oss", 70);                                                             //key for the message queue
    msgid = msgget(key, 0666);                                                                   //get the message queue

    key_t dataKey = 68;                                                             //key for the table in shared memory
    message.mesg_type = 1;                                                                  //set the shared memory type

    int end = 0;
    int dataId = shmget(dataKey, 2048, 0666|IPC_CREAT);                                //get the shared memory for table
    while(end == 0){                                                                                    //until end == 0
        char *killStr = (char *) shmat(killID, (void *) 0, 0);       //ptr to the shared memory location for process end
        strcpy(killStr, "Alive");                                 //copy the word Alive to the process end shared memory
        msgrcv(msgid, &message, sizeof(message), 1, 0);                           //receive the message in message queue
        table = shmat(dataId, NULL, 0);                                           //attach to the table in shared memory
        if(table[message.pidToRun].simPid == getpid()){       //if the pid for the process scheduled is the same as ours
            float quantum = message.timeQuantum;                                                  //get the time quantum
            table[message.pidToRun].burstTime = (float)(getSeconds() + getNano());                  //set the burst time
            end = work(table, quantum);                                        //set end to the value returned from work
        }
        shmdt(table);                                                      //detach from the shared memory for the table
        shmdt(killStr);                                                    //detach from the shared memory for child end
    }
    char *killedStr = (char *) shmat(killID, (void *) 0, 0);                //ptr to the shared memory for the child end
    strcpy(killedStr, "Killed");                                                  //write "Killed" to that shared memory
    shmdt(killedStr);                                                      //detach from the shared memory for child end
    shmdt(table);                                                          //detach from the shared memory for the table
    strcpy(message.mesg_text, "received");                                       //change the message text to "received"
    msgsnd(msgid, &message, sizeof(message), 0);                                                 //send the message back

    exit(0);
}