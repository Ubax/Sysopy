#include <stdio.h>
#include <unistd.h>
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#ifndef SYSOPY_COMMUNICATION_H
#define SYSOPY_COMMUNICATION_H


#define ERROR_EXIT(msg){perror(msg);exit(1);}
#define MESSAGE_EXIT(msg, ...){printf(msg, ##__VA_ARGS__);exit(0);}

#define MESSAGE_SIZE 2048
#define MAX_COMMAND_LENGTH MESSAGE_SIZE
#define MAX_COMMAND_ID 15
#define MAX_NUMBER_OF_CLIENTS 20
#define LIST_DELIMITER ";"
#define MAX_QUEUE_SIZE 9
#define SERVER_QUEUE_NAME "/serverQueue"

struct MESSAGE {
    long mType;
    pid_t senderId;
    char message[MESSAGE_SIZE];
};


enum COMMAND {
    STOP = 1,
    INIT,
    LIST,
    FRIENDS,
    ADD,
    DEL,
    ECHO,
    _2ONE,
    _2FRIENDS,
    _2ALL,
};

unsigned commandPiority(enum COMMAND cmd){
    switch(cmd){
        case STOP:
            return 4;
        case INIT:
            return 3;
        case FRIENDS:
        case ADD:
        case DEL:
        case LIST:
            return 2;
        default:
            return 1;
    }
}

char* getClientQueueName() {
    char *name=malloc(32*sizeof(char));
    sprintf(name, "/cli%i%i", getpid(), rand()%1000);
    return name;
}

#define MSGSZ sizeof(struct MESSAGE) //msgsz doesn't contain mType

#endif //SYSOPY_COMMUNICATION_H
