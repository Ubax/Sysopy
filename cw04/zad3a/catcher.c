#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "catcherLib.h"

void printWelcome();
void printGoodbye(int numberOfSignals);

int main(int argc, char **argv) {
    printWelcome();
    size_t i =0;

    for(;i<31;i++){
        if(i!=SIGUSR1 && i!=SIGUSR2)signal(i, SIG_IGN);
    }
    int numberOfSignals = receive();
    pid_t senderPID = getSenderPid();
    i=0;
    for(;i<numberOfSignals;i++){
        kill(senderPID, SIGUSR1);
    }
    kill(senderPID, SIGUSR2);
    printGoodbye(numberOfSignals);
    return 0;
}

void printWelcome() {
    printf("PID:\t%i\n", getpid());
}

void printGoodbye(int numberOfSignals){
    printf("Number of received signals:\t%i\n", numberOfSignals);
}