//
// Created by ubax on 19.04.19.
//

#ifndef SYSOPY_COMMUNICATION_H
#define SYSOPY_COMMUNICATION_H


#define ERROR_EXIT(msg){perror(msg);exit(1);}
#define MESSAGE_EXIT(msg, ...){printf(msg, ##__VA_ARGS__);exit(1);}

#define MESSAGE_SIZE 2048
#define MAX_COMMAND_LENGTH 1024
#define MAX_COMMAND_ID 10
#define KEY_LETTER 'a'

struct MESSAGE{
    long mType;
    pid_t senderPid;
    char message[MESSAGE_SIZE];
};


enum COMMAND{
    STOP = MAX_COMMAND_ID - 6,
    LIST = MAX_COMMAND_ID - 5,
    FRIENDS = MAX_COMMAND_ID - 4,
    ECHO = MAX_COMMAND_ID - 3,
    _2ONE = MAX_COMMAND_ID - 2,
    _2FRIENDS = MAX_COMMAND_ID - 1,
    _2ALL = MAX_COMMAND_ID,
};


key_t getServerQueueKey(){
    char* homeDir = getenv("HOME");
    if(homeDir==NULL)MESSAGE_EXIT("No home environment variable");
    key_t key = ftok(homeDir,KEY_LETTER);
    if(key==-1)ERROR_EXIT("Generating key");
    return  key;
}

#define MSGSZ sizeof(struct MESSAGE)-sizeof(long) //msgsz doesn't contain mType

#endif //SYSOPY_COMMUNICATION_H
