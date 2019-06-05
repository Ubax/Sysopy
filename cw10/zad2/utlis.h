#ifndef _UTILS_H
#define _UTILS_H

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>

#define UNIX_PATH_MAX 108
#define MAX_CLIENTS 10

#define debug(format, ...) { printf(format, ##__VA_ARGS__); putchar('\n'); }

typedef enum sock_msg_type_t {
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
} sock_msg_type_t;

typedef struct sock_msg_t {
    uint8_t type;
    uint64_t size;
    uint64_t name_size;
    uint64_t id;
    void *content;
    char *name;
} sock_msg_t;

typedef struct client_t {
    int fd;
    char *name;
    struct sockaddr *addr;
    socklen_t addr_len;
    uint8_t working;
    uint8_t inactive;
} client_t;

int parse_pos_int(char* s)
{
    int i = atoi(s);
    if (i <= 0) {
        fprintf(stderr, "%s should be a positive integer\n", s);
        exit(-1);
    } else {
        return i;
    }
}

void show_errno(void)
{
    fputs(strerror(errno), stderr);
}
void die(char* msg)
{
    assert(msg);
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void die_errno(void)
{
    die(strerror(errno));
}



#endif