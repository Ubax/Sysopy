#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "catcherLib.h"

int receiving = 1;
int numberOfSignals = 0;
pid_t senderPID;
int _isQueue = 0;
int _lastIntValue=-1;

void sigUsr1(int sig, siginfo_t *info, void *ucontext) {
    if(receiving){
        numberOfSignals++;
        printf("%i\n",info->si_value.sival_int);
        _lastIntValue=info->si_value.sival_int;
    }
}

void sigUsr2(int sig, siginfo_t *info, void *ucontext) {
    receiving = 0;
    senderPID=info->si_pid;
    if(info->si_value.sival_int==getpid()){
        _isQueue=1;
    }
}

pid_t getSenderPid(){
    return senderPID;
}

int isQueue(){
    return _isQueue;
}

int getLastIntValue(){
    return _lastIntValue;
}

int receive(){
    struct sigaction actUsr2;
    sigemptyset(&actUsr2.sa_mask);
    actUsr2.sa_flags=SA_SIGINFO;
    actUsr2.sa_sigaction=&sigUsr2;

    struct sigaction actUsr1;
    sigemptyset(&actUsr1.sa_mask);
    actUsr1.sa_flags=SA_SIGINFO;
    actUsr1.sa_sigaction=&sigUsr1;

    sigaction(SIGUSR2, &actUsr2, NULL);
    sigaction(SIGUSR1, &actUsr1, NULL);
    sigaction(SIG_REAL_USR2, &actUsr2, NULL);
    sigaction(SIG_REAL_USR1, &actUsr1, NULL);

    while (receiving) {
        sleep(1);
    }
    return numberOfSignals;
}


