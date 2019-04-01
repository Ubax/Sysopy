#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "argsProcessor.h"
#include "senderLib.h"

enum TYPE getTypeFormArgv(char ** argv, int i);
void on_sigusr1(int sig, siginfo_t *info, void *ucontext);
int sender(pid_t pid, enum TYPE type, size_t numberOfSignals);

int waiting=0;
int numberOfReceivedSignals = 0;
size_t numberOfSignals = 0;

int main(int argc, char **argv) {

    if(argc<4){
        printf("Program expects at last 3 argument: [catcher PID] [number of signals] [method of sending]\n");
        return 1;
    }
    pid_t catcherPID=getArgAsInt(argv, 1);
    if(catcherPID<2){
        printf("Wrong catcher pid\n");
        return 1;
    }
    numberOfSignals=getArgAsSizeT(argv, 2);

    enum TYPE type = getTypeFormArgv(argv, 3);

    if(sender(catcherPID, type, numberOfSignals)){
        perror("sender");
        return 1;
    }

    return 0;
}

int initSignals(void (*fun)(int, siginfo_t *, void *)){
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = fun;

    if (sigaction(SIGUSR1, &act, NULL) != 0 || sigaction(SIGUSR2, &act, NULL) != 0)
    {
        perror("Init Signals");
        exit(1);
    }
    return 0;
}

int initRTSignals(void (*fun)(int, siginfo_t *, void *)){
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = fun;

    if (sigaction(SIG_REAL_USR1, &act, NULL) != 0 || sigaction(SIG_REAL_USR2, &act, NULL) != 0)
    {
        perror("Init Signals");
        exit(1);
    }
    return 0;
}

void onSigKill(int sig, siginfo_t *info, void *ucontext) {
    if(sig==SIGUSR1){
        numberOfReceivedSignals++;
        printf("Received answer\n");
        waiting=0;
    }else if(sig==SIGUSR2){
        if(numberOfReceivedSignals<numberOfSignals){
            printf("Expected: %ld\tGot: %i\n", numberOfSignals, numberOfReceivedSignals);
            exit(1);
        }
    }
}

void onSigRT(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIG_REAL_USR1) {
        numberOfReceivedSignals++;
        printf("Received answer\n");
        waiting=0;
    } else if (sig == SIG_REAL_USR2) {
        if(numberOfReceivedSignals<numberOfSignals){
            printf("Expected: %ld\tGot: %i\n", numberOfSignals, numberOfReceivedSignals);
            exit(1);
        }
    }
}

int sender(pid_t pid, enum TYPE type, size_t numberOfSignals) {
    if(type==KILL)initSignals(&onSigKill);
    if(type==SIGQUEUE)initSignals(&onSigKill);
    else if(type==SIGRT)initRTSignals(&onSigRT);

    size_t i=0;
    for(;i<numberOfSignals;i++){
        int ret = send(pid, type, SIG_SIGUSR1, i);
        if(ret!=0)return ret;
        waiting=1;
        while(waiting){sleep(1);}
    }
    return send(pid, type, SIG_SIGUSR2, i);
}


enum TYPE getTypeFormArgv(char ** argv, int i){
    enum TYPE type;
    if(compareArg(argv, i, "kill"))type = KILL;
    else if(compareArg(argv, i, "sigqueue"))type = SIGQUEUE;
    else if(compareArg(argv, i, "sigrt"))type = SIGRT;
    else{
        printf("Unknown sending method\n");
        exit(1);
    }
    return type;
}

