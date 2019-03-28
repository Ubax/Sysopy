#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "catcherLib.h"

int receiving = 1;
int numberOfSignals = 0;
pid_t senderPID;
int shouldPrint = 0;
int lastVal = 0;

void sigUsr1(int sig, siginfo_t *info, void *ucontext) {
    if (receiving) {
        numberOfSignals++;
        if (shouldPrint)printf("%i\n", info->si_value.sival_int);
        lastVal = info->si_value.sival_int;
    }
}

void sigUsr2(int sig, siginfo_t *info, void *ucontext) {
    receiving = 0;
    senderPID = info->si_pid;
}

pid_t getSenderPid() {
    return senderPID;
}

int getLastVal() {
    return lastVal;
}

int receive(enum TYPE type) {
    switch (type) {
        case SIGRT:
        case KILL:
            shouldPrint = 0;
            break;
        case SIGQUEUE:
            shouldPrint = 0;
            break;
    }
    struct sigaction actUsr2;
    sigemptyset(&actUsr2.sa_mask);
    actUsr2.sa_flags = SA_SIGINFO;
    actUsr2.sa_sigaction = &sigUsr2;

    struct sigaction actUsr1;
    sigemptyset(&actUsr1.sa_mask);
    actUsr1.sa_flags = SA_SIGINFO;
    actUsr1.sa_sigaction = &sigUsr1;

    if (sigaction(SIGUSR2, &actUsr2, NULL) != 0)return -1;
    if (sigaction(SIGUSR1, &actUsr1, NULL) != 0)return -1;
    if (sigaction(SIG_REAL_USR2, &actUsr2, NULL) != 0)return -1;
    if (sigaction(SIG_REAL_USR1, &actUsr1, NULL) != 0)return -1;

    while (receiving) {
        //sleep(1);
    }
    return numberOfSignals;
}


