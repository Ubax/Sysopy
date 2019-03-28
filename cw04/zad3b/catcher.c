#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "argsProcessor.h"
#include "senderLib.h"

void printWelcome();
void printGoodbye(size_t numberOfSignals);
void lockSignals();
enum TYPE getTypeFormArgv(char ** argv, int i);
void sigUsr1(int sig, siginfo_t *info, void *ucontext);
void sigUsr2(int sig, siginfo_t *info, void *ucontext);

int receiving = 1;
int shouldSend = 0;
int numberOfSignals = 0;
pid_t senderPID;
enum TYPE type;

int main(int argc, char **argv) {
    if(argc<2){
        printf("Program expects at last 1 argument: [method of sending]\n");
        return 1;
    }
    enum TYPE type = getTypeFormArgv(argv, 1);
    lockSignals();
    printWelcome();

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
        if(shouldSend){
            printf("Sending signal\n");
            send(senderPID, type, SIG_SIGUSR1, 0);
            shouldSend=0;
        }
    }

    printGoodbye(numberOfSignals);
    return 0;
}

void sigUsr1(int sig, siginfo_t *info, void *ucontext) {
    if (receiving) {
        senderPID = info->si_pid;
        numberOfSignals++;
        printf("Received signal USR1\n");
        shouldSend=1;
    }
}

void sigUsr2(int sig, siginfo_t *info, void *ucontext) {
    receiving = 0;
    senderPID = info->si_pid;
}

void lockSignals(){
    size_t i =0;
    for(;i<31;i++){
        if(i!=SIGUSR1 && i!=SIGUSR2)signal((int)i, SIG_IGN);
    }
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

void printWelcome() {
    printf("PID:\t%i\n", getpid());
}

void printGoodbye(size_t numberOfSignals){
    printf("Number of received signals:\t%ld\n", numberOfSignals);
}