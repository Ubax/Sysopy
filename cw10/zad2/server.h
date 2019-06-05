#define MAX_CLIENTS_NUMBER 20
#define UNIX_PATH_MAX 80

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
#include "sockets.h"

#ifndef SERVER_H
#define SERVER_H

struct CLIENT {
    char *name;
    int socketFD;
    uint8_t working;
    uint8_t currently_working;
    uint8_t inactive;
    struct sockaddr *addr;
    socklen_t addr_len;
};



#endif
