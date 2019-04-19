#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>

#include "communication.h"

#define KEY_LETTER 'a'

#define ERROR_EXIT(msg){perror(msg);exit(1);}
#define MESSAGE_EXIT(msg, ...){printf(msg, ##__VA_ARGS__);exit(1);}

enum COMMAND{
    STOP = MAX_COMMAND_ID - 6,
    LIST = MAX_COMMAND_ID - 5,
    FRIENDS = MAX_COMMAND_ID - 4,
    ECHO = MAX_COMMAND_ID - 3,
    _2ONE = MAX_COMMAND_ID - 2,
    _2FRIENDS = MAX_COMMAND_ID - 1,
    _2ALL = MAX_COMMAND_ID,
};

int running = 1;

int processResponse(struct MESSAGE *msg);

int main(){
    char* homeDir = getenv("HOME");
    if(homeDir==NULL)MESSAGE_EXIT("No home environment variable");
    key_t key = ftok(homeDir,KEY_LETTER);
    if(key==-1)ERROR_EXIT("Generating key");
    int queueId = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
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