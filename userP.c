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

struct myTime updateTimeAfterDifference(struct myTime virtual, float difference){
    while(difference != 0){
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

struct myTime updateLocal(struct myTime temp){
    temp.nanoseconds = temp.nanoseconds + 30000;
    if(temp.nanoseconds >= 1000000000){
        temp.seconds++;
        temp.nanoseconds = temp.nanoseconds - 1000000000;
    }
    return temp;
}

int work(struct BLOCK *table, float quantum){
    int end = 1;
    srand(time(0));
    local.seconds = 0;
    local.nanoseconds = 0;
    int job = getRandom(100, 0);
    if(job < 5){
        job = 0;
        end = 2;
    }
    else if(job < 30){
        job = 2;
    }
    else if(job < 65){
        job = 1;
    }
    else{
        job = 3;
    }


    printf("Job: %d \n", job);
    if(job == 0){
        local.nanoseconds += 5000;
        printf("Process terminating at %f\n", (float)((float)getSeconds() + getNano()));
    }
    if(job == 1){
        while(1){
            local = updateLocal(local);
            if((float)(local.seconds + (float)((float)local.nanoseconds/1000000000)) >= quantum){
                break;
            }
        }
    }
    if(job == 2){
        int r = getRandom(5, 0);
        int s = getRandom(1000, 0);
        float difference = ((float)r + ((float)s / 10000));
        int currentSecond = local.seconds;
        float currentNano = local.nanoseconds/1000000000;
        while(1){
            if(((local.seconds + local.nanoseconds) - (float)(currentSecond + currentNano)) >= difference){
                updateTimeAfterDifference(local, difference);
                //block
                break;
            }
            local = updateLocal(local);
        }
    }
    if(job == 3){
        int p = getRandom(100, 1);
        float localCheck;
        while(1){
            localCheck = (float)((float)local.seconds + (float)(local.nanoseconds/1000000000));
            if(localCheck >= (float)((float)(p*quantum)/100)){
                break;
            }
            local = updateLocal(local);
        }
    }

    message.timeQuantum = (float)((float)local.seconds + (float)((float)local.nanoseconds/1000000000));
    table[message.pidToRun].CPUTime = (float)((float)local.seconds + (float)((float)local.nanoseconds/1000000000));
    table[message.pidToRun].run = 1;
    return end;
}

int main(int argc, char *argv[]){
    int msgid;
    key_t key = ftok("oss", 70);
    msgid = msgget(key, 0666);
    key_t dataKey = 68;
    message.mesg_type = 1;
    int end;
    int dataId = shmget(dataKey, 2048, 0666|IPC_CREAT);
    while(1){

        msgrcv(msgid, &message, sizeof(message), 1, 0);
        table = shmat(dataId, NULL, 0);
        /*if(table[message.pidToRun].simPid != getpid()){
            printf("WAITING     %d and %d\n", table[message.pidToRun].simPid, getpid());
        }*/
        if(table[message.pidToRun].simPid == getpid()){
            float quantum = message.timeQuantum;
            table[message.pidToRun].burstTime = (float)(getSeconds() + getNano());
            end = work(table, quantum);
            break;

        }
        shmdt(table);
    }
   shmdt(table);
    strcpy(message.mesg_text, "received");
    msgsnd(msgid, &message, sizeof(message), 0);

    exit(0);
}