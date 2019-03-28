#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "catcherLib.h"

int receiving = 1;
int numberOfSignals = 0;
pid_t senderPID;

void sigUsr1(int signalno) {
    if(receiving)numberOfSignals++;
}

void sigUsr2(int sig, siginfo_t *info, void *ucontext) {
    receiving = 0;
    senderPID=info->si_pid;
}

pid_t getSenderPid(){
    return senderPID;
}

int receive(){
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags=SA_SIGINFO;
    act.sa_sigaction=&sigUsr2;

    sigaction(SIGUSR2, &act, NULL);
    signal(SIGUSR1, &sigUsr1);
    signal(SIGUSR1, &sigUsr1);

    sigaction(SIGUSR2, &act, NULL);
    signal(SIGUSR1, &sigUsr1);

    while (receiving) {
        sleep(1);
    }
    return numberOfSignals;
}


