#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "argsProcessor.h"
#include "catcherLib.h"
#include "senderLib.h"

void printWelcome();
void printGoodbye(size_t numberOfSignals);
void lockSignals();
enum TYPE getTypeFormArgv(char ** argv, int i);

int main(int argc, char **argv) {
    if(argc<2){
        printf("Program expects at last 1 argument: [method of sending]\n");
        return 1;
    }
    enum TYPE type = getTypeFormArgv(argv, 1);

    lockSignals();

    printWelcome();

    size_t numberOfSignals = (size_t)receive(type);
    pid_t senderPID = getSenderPid();

    if(sender(senderPID, type, numberOfSignals)!=0){
        perror("SIGNAL ERROR");
    };

    printGoodbye(numberOfSignals);
    return 0;
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