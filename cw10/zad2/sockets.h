#ifndef SYSOPY_SOCKETS_H
#define SYSOPY_SOCKETS_H

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
    STOP,
};

struct SOCKET_MSG {
    uint8_t type;
    uint64_t size;
    uint64_t name_size;
    uint64_t id;
    void *content;
    char *name;
};

#endif //SYSOPY_SOCKETS_H
