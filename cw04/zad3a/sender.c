#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "argsProcessor.h"
#include "senderLib.h"

enum TYPE getTypeFormArgv(char **argv, int i);

size_t numberOfReceivedSignals = 0;
int receiving = 1;
int lastValue = -1;

void onSigKill(int sig, siginfo_t *info, void *ucontext);
void onSigRT(int sig, siginfo_t *info, void *ucontext);
int initSignals(void (*fun)(int, siginfo_t *, void *));
int initRTSignals(void (*fun)(int, siginfo_t *, void *));

int main(int argc, char **argv) {
    printf("A\n");
    if (argc < 4) {
        printf("Program expects at last 3 argument: [catcher PID] [number of signals] [method of sending]\n");
        return 1;
    }
    pid_t catcherPID = getArgAsInt(argv, 1);
    if (catcherPID < 2) {
        printf("Wrong catcher pid\n");
        return 1;
    }
    size_t numberOfSignals = getArgAsSizeT(argv, 2);

    enum TYPE type = getTypeFormArgv(argv, 3);

    if(type==KILL)initSignals(&onSigKill);
    else if(type==SIGQUEUE)initSignals(&onSigKill);
    else if(type==SIGRT)initRTSignals(&onSigRT);

    size_t i = 0;
    for (; i < numberOfSignals; i++) {
        if (send(catcherPID, type, SIG_SIGUSR1, i)) {
            perror("Sending signal");
            break;
        }
    }

    if (send(catcherPID, type, SIG_SIGUSR2, 0)) {
        perror("Sending signal");
        exit(1);
    }

    while (receiving) {
        sleep(1);
    }
    printf("Received signals:\t%ld\nSent signals:\t%ld\n", numberOfReceivedSignals, numberOfSignals);
    if (type == SIGQUEUE)printf("Catcher max number:\t%i\n", lastValue);

    return 0;
}

void onSigKill(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1) {
        if (receiving) {
            numberOfReceivedSignals++;
            lastValue = info->si_value.sival_int;
        }
    } else if (sig == SIGUSR2) {
        receiving = 0;
    }
}

void onSigRT(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIG_REAL_USR1) {
        if (receiving) {
            numberOfReceivedSignals++;
        }
    } else if (sig == SIG_REAL_USR2) {
        receiving = 0;
    }
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

enum TYPE getTypeFormArgv(char **argv, int i) {
    enum TYPE type;
    if (compareArg(argv, i, "kill"))type = KILL;
    else if (compareArg(argv, i, "sigqueue"))type = SIGQUEUE;
    else if (compareArg(argv, i, "sigrt"))type = SIGRT;
    else {
        printf("Unknown sending method\n");
        exit(1);
    }
    return type;
}

