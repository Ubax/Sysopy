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
#define MESSAGE_EXIT(msg, ...){printf(msg, ##__VA_ARGS__);exit(1);}

#define MESSAGE_SIZE 2048
#define MAX_COMMAND_LENGTH MESSAGE_SIZE
#define MAX_COMMAND_ID 15
#define KEY_LETTER 'a'
#define MAX_NUMBER_OF_CLIENTS 20
#define LIST_DELIMITER ";"
#define MAX_QUEUE_SIZE 100
#define SERVER_QUEUE_NAME "serverQueue"

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


key_t getServerQueueKey() {
    char *homeDir = getenv("HOME");
    if (homeDir == NULL) MESSAGE_EXIT("No home environment variable");
    key_t key = ftok(homeDir, KEY_LETTER);
    if (key == -1) ERROR_EXIT("Generating key");
    return key;
}

key_t getClientQueueKey() {
    char *homeDir = getenv("HOME");
    if (homeDir == NULL) MESSAGE_EXIT("No home environment variable");
    key_t key = ftok(homeDir, getpid());
    if (key == -1) ERROR_EXIT("Generating key");
    return key;
}

#define MSGSZ sizeof(struct MESSAGE) //msgsz doesn't contain mType

#endif //SYSOPY_COMMUNICATION_H
