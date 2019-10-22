#include "sharedMem.h"

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
    temp.nanoseconds = temp.nanoseconds + 10000;
    if(temp.nanoseconds >= 1000000000){
        temp.seconds++;
        temp.nanoseconds = temp.nanoseconds - 1000000000;
    }
    return temp;
}

void work(struct BLOCK *table, float quantum){
    srand(time(0));
    local.seconds = 0;
    local.nanoseconds = 0;
    int job = getRandom(3, 0);
    printf("Job: %d \n", job);
    if(job == 0){
        printf("Process terminating at %f\n", (float)((float)getSeconds() + getNano()));
    }
    if(job == 1){
        while(1){
            local = updateLocal(local);
            if((float)(local.seconds + (float)(local.nanoseconds/1000000000)) >= quantum){
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
                printf("Waited for event that lasted %f\n", difference);
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
                printf("Local: %f | Quantum Percentage: %f\n", localCheck, (float)((float)(p*quantum)/100));
                break;
            }
            local = updateLocal(local);
        }
    }

    message.timeQuantum = (float)((float)local.seconds + (float)(local.nanoseconds/1000000000));

}

int main(int argc, char *argv[]){
    int msgid;
    key_t key = ftok("oss", 70);
    msgid = msgget(key, 0666);
    msgrcv(msgid, &message, sizeof(message), 1, 0);
    printf("Data Received is : %s \n",
           message.mesg_text);
    float quantum = message.timeQuantum;

    key_t dataKey = 68;
    int dataId = shmget(dataKey, 2048, 0666|IPC_CREAT);
    table = shmat(dataId, NULL, 0);
    work(table, quantum);
    shmdt(table);
    strcpy(message.mesg_text, "received");
    msgsnd(msgid, &message, sizeof(message), 0);

    exit(0);
}