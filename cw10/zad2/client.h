#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "sysopy.h"

#define UNIX_PATH_MAX 80

#ifndef CLIENT_H
#define CLIENT_H

enum CONNECTION_TYPE {
    NETWORK, UNIX
};

enum SOCKET_MSG_TYPE {
    REGISTER,
    UNREGISTER,
    PING,
    PONG,
    OK,
    NAME_TAKEN,
    FULL,
    FAIL,
    WORK,
    WORK_DONE,
};

struct SOCKET_MSG {
    uint8_t type;
    uint64_t size;
    uint64_t name_size;
    uint64_t id;
    void *content;
    char *name;
};


#endif
