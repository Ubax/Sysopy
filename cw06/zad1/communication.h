//
// Created by ubax on 19.04.19.
//

#ifndef SYSOPY_COMMUNICATION_H
#define SYSOPY_COMMUNICATION_H

#define MESSAGE_SIZE 2048
#define MAX_COMMAND_ID 10

struct MESSAGE{
    long mType;
    pid_t senderPid;
    char message[MESSAGE_SIZE];
};

#define MSGSZ sizeof(struct MESSAGE)-sizeof(long) //msgsz doesn't contain mType

#endif //SYSOPY_COMMUNICATION_H
