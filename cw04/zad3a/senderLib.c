#include <signal.h>
#include <errno.h>
#include "senderLib.h"
#include "catcherLib.h"

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

int send(pid_t pid, enum TYPE type, enum SIGNAL signalType) {
    switch (type) {
        case KILL:
            return sendKill(pid, signalType);
        case SIGQUEUE:

            break;
        case SIGRT:
            return sendSigrt(pid, signalType);
    }
    return 0;
}

int sender(pid_t pid, enum TYPE type, size_t numberOfSignals) {
    size_t i=0;
    for(;i<numberOfSignals;i++){
        send(pid, type, SIG_SIGUSR1);
    }
    send(pid, type, SIG_SIGUSR2);
    return 0;
}