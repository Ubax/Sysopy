#ifndef SYSOPY_SENDERLIB_H
#define SYSOPY_SENDERLIB_H


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

int send(pid_t pid, enum TYPE type, enum SIGNAL signalType);
int sender(pid_t pid, enum TYPE type, size_t numberOfSignals);

#endif //SYSOPY_SENDERLIB_H
