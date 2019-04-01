#ifndef SYSOPY_SENDERLIB_H
#define SYSOPY_SENDERLIB_H

#define SIG_REAL_USR1 SIGRTMIN+3
#define SIG_REAL_USR2 SIGRTMIN+4

enum SIGNAL {
    SIG_SIGUSR1,
    SIG_SIGUSR2,
    SIG_SIGREAL1,
    SIG_SIGREAL2,
};

enum TYPE {
    KILL,
    SIGQUEUE,
    SIGRT
};

int send(pid_t pid, enum TYPE type, enum SIGNAL signalType, int order);

#endif //SYSOPY_SENDERLIB_H
