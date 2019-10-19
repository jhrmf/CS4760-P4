#include "sharedMem.h"

int main(int argc, char *argv[]){
    int msgid;

    key_t key = ftok("oss", 70);

    msgid = msgget(key, 0666);


    msgrcv(msgid, &message, sizeof(message), 1, 0);

    printf("Data Received is : %s \n",
           message.mesg_text);

    strcpy(message.mesg_text, "received");
    msgsnd(msgid, &message, sizeof(message), 0);

    exit(0);
}