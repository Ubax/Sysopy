#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "argsProcessor.h"

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

int sendKill(pid_t pid, enum SIGNAL signalType) {
    switch (signalType) {
        case SIG_SIGUSR1:
            if(kill(pid, SIGUSR1)!=0)return errno;
            break;
        case SIG_SIGUSR2:
            if(kill(pid, SIGUSR2)!=0)return errno;
            break;
        case SIG_SIGREAL1:
            if(kill(pid, SIGRTMIN+1)!=0)return errno;
            break;
        case SIG_SIGREAL2:
            if(kill(pid, SIGRTMIN+2)!=0)return errno;
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

int main(int argc, char **argv) {
    enum TYPE type;
    if(argc<4){
        printf("Program expects at last 3 argument: [catcher PID] [number of signals] [method of sending]\n");
        return 1;
    }
    pid_t catcherPID=getArgAsInt(argv, 1);
    if(catcherPID<2){
        printf("Wrong catcher pid\n");
        return 1;
    }
    size_t numberOfSignals=getArgAsSizeT(argv, 2);

    if(compareArg(argv, 3, "kill"))type = KILL;
    else if(compareArg(argv, 3, "sigqueue"))type = SIGQUEUE;
    else if(compareArg(argv, 3, "sigrt"))type = SIGRT;
    else{
        printf("Unknown sending method\n");
        return 1;
    }

    sender(catcherPID, type, numberOfSignals);

    return 0;
}