#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "argsProcessor.h"
#include "senderLib.h"
#include "inits.h"

void printWelcome();

void printGoodbye(size_t numberOfSignals);

enum TYPE getTypeFormArgv(char **argv, int i);

void onSigKill(int sig, siginfo_t *info, void *ucontext);
void onSigRT(int sig, siginfo_t *info, void *ucontext);

int receiving = 1;
size_t numberOfSignals = 0;
pid_t senderPID;
enum TYPE type;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Program expects at last 1 argument: [method of sending]\n");
        return 1;
    }
    type = getTypeFormArgv(argv, 1);
    printWelcome();

    if(type==KILL)initSignals(&onSigKill);
    else if(type==SIGQUEUE)initSignals(&onSigKill);
    else if(type==SIGRT)initRTSignals(&onSigRT);

    while (receiving) {
        sleep(1);
    }

    printGoodbye(numberOfSignals);
    return 0;
}

void onSigKill(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIGUSR1) {
        if (receiving) {
            senderPID = info->si_pid;
            numberOfSignals++;
            printf("Received signal USR1\n");
            printf("Sending signal\n");
            if(send(senderPID, type, SIG_SIGUSR1, 0)){
                perror("Sending signal");
                exit(1);
            }
        }
    } else if (sig == SIGUSR2) {
        receiving = 0;
    }
}

void onSigRT(int sig, siginfo_t *info, void *ucontext) {
    if (sig == SIG_REAL_USR1) {
        if (receiving) {
            senderPID = info->si_pid;
            numberOfSignals++;
            printf("Received signal RT USR1\n");
            printf("Sending signal\n");
            if(send(senderPID, type, SIG_SIGUSR1, 0)){
                perror("Sending signal");
                exit(1);
            }
        }
    } else if (sig == SIG_REAL_USR2) {
        receiving = 0;
    }
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

void printWelcome() {
    printf("PID:\t%i\n", getpid());
}

void printGoodbye(size_t numberOfSignals) {
    printf("Number of received signals:\t%ld\n", numberOfSignals);
}