#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>

#include "communication.h"

int running = 1;

int processResponse(struct MESSAGE *msg);

int main(){
    int queueId = msgget( getServerQueueKey(), IPC_CREAT | IPC_EXCL | 0666);
    if(queueId==-1)ERROR_EXIT("Creating queue");
    struct MESSAGE msgbuf;
    while (running){
        if(msgrcv(queueId, &msgbuf, MSGSZ, -(MAX_COMMAND_ID+1), 0)==-1)ERROR_EXIT("Receiving");
        processResponse(&msgbuf);
    }
    if(msgctl(queueId, IPC_RMID, NULL)==-1)ERROR_EXIT("Deleting queue");
    return 0;
}

int processResponse(struct MESSAGE *msg){
    if(msg->mType==STOP){
        running=0;
        return 0;
    }
    printf("%s\n", msg->message);
    return 0;
}