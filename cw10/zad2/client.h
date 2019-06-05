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

#define UNIX_PATH_MAX 80

#ifndef CLIENT_H
#define CLIENT_H

enum CONNECTION_TYPE {
    NETWORK, UNIX
};


#endif
