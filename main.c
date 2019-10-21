#include "sharedMem.h"

typedef struct node {
    int data;
    int priority;
    struct node* next;

} Node;

Node* newNode(int d, int p)
{
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->data = d;
    temp->priority = p;
    temp->next = NULL;

    return temp;
}

void dequeue(Node** head)
{
    Node* temp = *head;
    (*head) = (*head)->next;
    free(temp);
}

// Function to push according to priority
void enqueue(Node** head, int d, int p)
{
    Node* start = (*head);

    // Create new Node
    Node* temp = newNode(d, p);

    // Special Case: The head of list has lesser
    // priority than new node. So insert new
    // node before head node and change head node.
    if ((*head)->priority > p) {

        // Insert New Node before head
        temp->next = *head;
        (*head) = temp;
    }
    else {

        // Traverse the list and find a
        // position to insert new node
        while (start->next != NULL &&
               start->next->priority < p) {
            start = start->next;
        }

        // Either at the ends of the list
        // or at required position
        temp->next = start->next;
        start->next = temp;
    }
}

struct myTime updateTimeAfterProcess(struct myTime virtual, float quantum){
    while(quantum != 0){
        if(quantum >= 1){
            virtual.seconds++;
            quantum--;
        }
        else{
            virtual.nanoseconds += quantum;
            break;
        }
    }
    return virtual;
}

struct myTime updateClock(struct myTime virtual){     //function for updating the system clock's seconds and nanoseconds
    virtual.nanoseconds = virtual.nanoseconds + 80000;                       //increment nanoseconds by 40000 every call
    if(virtual.nanoseconds >= 1000000000){                          //if the nanoseconds exceeds or is equal to a second
        virtual.seconds++;                                                             //increment the seconds counter
        virtual.nanoseconds = virtual.nanoseconds - 1000000000;             //decrement the nanosecond count by a second
    }
    int tempNano = virtual.nanoseconds;
    int tempSec = virtual.seconds;
    key_t secKey = 66;                                         //the key for the shared memory holding the seconds is 66
    key_t nanoKey = 67;                                    //the key for the shared memory holding the nanoseconds is 67
    int secID = shmget(secKey, 2048, 0666|IPC_CREAT);    //access the shared memory with file write and read permissions
    int nanoID = shmget(nanoKey, 2048, 0666|IPC_CREAT); //same as above, secID is for seconds, nanoID is for nanoseconds
    char temp1[10], temp2[11];                                               //two temp strings for holding time
    sprintf(temp1, "%d", virtual.seconds);                                       //store the seconds in a string
    sprintf(temp2, "%d", virtual.nanoseconds);                               //store the nanoseconds in a string
    char *secStr = (char *) shmat(secID, (void *) 0, 0);         //ptr to the shared memory location for seconds
    strcpy(secStr, temp1);                                      //copy the seconds to the shared memory location
    shmdt(secStr);                                                      //detach from the shared memory location
    char *nanoStr = (char *) shmat(nanoID, (void *) 0, 0);   //ptr to the shared memory location for nanoseconds
    strcpy(nanoStr, temp2);                                 //copy the nanoseconds to the shared memory location
    shmdt(nanoStr);                                                     //detach from the shared memory location
    virtual.nanoseconds = tempNano;
    virtual.seconds = tempSec;
    return virtual;                                           //return the new time stored in the virtual time structure
}

float getLaunchTime(int maxTimeBetweenNewProcsNS, int maxTimeBetweenNewProcsSecs){
    float launchTime;
    if(maxTimeBetweenNewProcsNS != 0){
        launchTime = getRandom(maxTimeBetweenNewProcsNS, 0) / 1000000000;
    }
    if( maxTimeBetweenNewProcsSecs != 0){
        launchTime = launchTime + getRandom(maxTimeBetweenNewProcsSecs, 0);
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


    int timeInSec = 10;                                                               //default timer is set to 5 seconds
    int maxChildren = 5;                                                       //default max child processes is set to 5
    int i = 0;
    int maxTimeBetweenNewProcsNS = 1000000000, maxTimeBetweenNewProcsSecs = 0;
    float launchItUp = getLaunchTime(maxTimeBetweenNewProcsNS, maxTimeBetweenNewProcsSecs);
    char fileName[100] = "logFile";                                            //default log file name is set to logFile
    virtual.nanoseconds = 0;                                                            //nanosecond counter is set to 0
    virtual.seconds = 0;                                                                    //second counter is set to 0

    setupinterrupt();                                                    //start the interrupt and timer for the program
    setupitimer(timeInSec);

    int childCount;                                      //this is for a loop below to hold the count of child processes
    pid_t childPid;                                            //of course we need one of these guys for child processes

    key_t dataKey = 68;
    int dataId = shmget(dataKey, 2048, 0666|IPC_CREAT);

    key_t secKey = 66;                                         //the key for the shared memory holding the seconds is 66
    key_t nanoKey = 67;                                    //the key for the shared memory holding the nanoseconds is 67
    int secID = shmget(secKey, 2048, 0666|IPC_CREAT);    //access the shared memory with file write and read permissions
    int nanoID = shmget(nanoKey, 2048, 0666|IPC_CREAT); //same as above, secID is for seconds, nanoID is for nanoseconds

   /* table = shmat(dataId, NULL, 0);
    table[0].CPUTime = 10;
    shmdt(table);
    struct BLOCK *test;
    test = shmat(shmget(dataKey, 2048, 0444), NULL, 0);
    printf("CPU Time is %d\n", test[0].CPUTime);
    printf("CPU Time for other is %d\n", test[1].CPUTime);
    shmctl(dataId, IPC_RMID, NULL);
*/
    int msgid;
    while(virtual.seconds < 5 && childCount <= 100){   //this do while loop ends if seconds < 3 or 100 children exist
        tempTime = updateClock(tempTime);
        float checkTime = tempTime.seconds + ((float)tempTime.nanoseconds/1000000000);
        if(checkTime >= 1) {
            printf("Launched at %f!\n", checkTime);
            tempTime.seconds = 0;
            tempTime.nanoseconds = 0;

            if (i < 18) {
                key_t key = ftok("oss", 70);
                msgid = msgget(key, 0666 | IPC_CREAT);
                message.mesg_type = 1;
                strcpy(message.mesg_text, "PROCESS ENTERED SYSTEM");
                message.timeQuantum = 1;
                msgsnd(msgid, &message, sizeof(message), 0);
                printf("----------NEW PROCESS----------\n");
                childPid = fork();
            }
            if(childPid == 0){
                table = shmat(dataId, NULL, 0);
                table[i].CPUTime = 0.0;
                table[i].sysTime = (float)((float)virtual.seconds + (float)(virtual.nanoseconds/1000000000));
                table[i].burstTime = 0.0;
                table[i].simPid = getpid();
                shmdt(table);
                execl("./userP", NULL);
                exit(0);
            }
            else{
                i++;
                sleep(1);
                while (strcmp(message.mesg_text, "sent") == 0 && virtual.nanoseconds <= 10) {
                    msgrcv(msgid, &message, sizeof(message), 1, 0);
                    virtual.nanoseconds++;
                    if(virtual.nanoseconds >= 1000000000){
                        virtual.nanoseconds -= 1000000000;
                        virtual.seconds++;
                    }
                }
                printf("Time Prior to Process: %f \n",(float)(virtual.seconds + (float)(virtual.nanoseconds/1000000000)));
                printf("Time of Process: %f\n", message.timeQuantum);
                virtual = updateTimeAfterProcess(virtual, message.timeQuantum);
                printf("Updated Time: %f\n", (float)(virtual.seconds + (float)(virtual.nanoseconds/1000000000)));
            }

        }else{
            virtual = updateClock(virtual);
        }
    }

    shmctl(secID, IPC_RMID, NULL);                                                //delete the shared memory for seconds
    shmctl(nanoID, IPC_RMID, NULL);                                           //delete the shared memory for nanoseconds
    msgctl(msgid, IPC_RMID, NULL);

    table = shmat(dataId, NULL, 0);
    for(i=18; i > -1; i--){
        printf("CPU Time: %f\n", table[i].CPUTime);
        printf("System Time: %f\n", table[i].sysTime);
        printf("Burst Time: %f\n", table[i].burstTime);
        printf("Simulated PID: %d\n", table[i].simPid);
        printf("\n");
    }
    shmdt(table);

    shmctl(dataId, IPC_RMID, NULL);
    exit(0);                                                                    // end the process, cause its the parent

}