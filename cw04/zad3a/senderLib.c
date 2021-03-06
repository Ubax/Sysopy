#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "senderLib.h"

int sendKill(pid_t pid, enum SIGNAL signalType) {
    switch (signalType) {
        case SIG_SIGUSR1:
            if(kill(pid, SIGUSR1)!=0)return errno;
            break;
        case SIG_SIGUSR2:
            if(kill(pid, SIGUSR2)!=0)return errno;
            break;
        case SIG_SIGREAL1:
            if(kill(pid, SIG_REAL_USR1)!=0)return errno;
            break;
        case SIG_SIGREAL2:
            if(kill(pid, SIG_REAL_USR2)!=0)return errno;
            break;
    }
    return 0;
}

int sendSigrt(pid_t pid, enum SIGNAL signalType) {
    switch (signalType) {
        case SIG_SIGUSR1:
            return sendKill(pid, SIG_SIGREAL1);
        case SIG_SIGUSR2:
            return sendKill(pid, SIG_SIGREAL2);
        default:
            return 1;
    }
    return 0;
}

int sendQueue(pid_t pid, enum SIGNAL signalType, int i) {
    union sigval val;
    val.sival_int=i;
    switch (signalType) {
        case SIG_SIGUSR1:
            if(sigqueue(pid, SIGUSR1, val)!=0)return errno;
            break;
        case SIG_SIGUSR2:
            if(sigqueue(pid, SIGUSR2, val)!=0)return errno;
            break;
        default:
            return 1;
    }
    return 0;
}

int send(pid_t pid, enum TYPE type, enum SIGNAL signalType, int order) {
    switch (type) {
        case KILL:
            return sendKill(pid, signalType);
        case SIGQUEUE:
            return sendQueue(pid, signalType, order);
        case SIGRT:
            return sendSigrt(pid, signalType);
    }
    return 0;
}