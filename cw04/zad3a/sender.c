#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include "argsProcessor.h"
#include "senderLib.h"
#include "catcherLib.h"

enum TYPE getTypeFormArgv(char ** argv, int i);

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
    size_t numberOfSignals=getArgAsSizeT(argv, 2);

    enum TYPE type = getTypeFormArgv(argv, 3);

    if(sender(catcherPID, type, numberOfSignals)){
        perror("sender");
        return 1;
    }
    int numberOfReceivedSignals = receive(type);
    printf("Received signals:\t%i\nSent signals:\t%ld\n", numberOfReceivedSignals, numberOfSignals);
    if(type==SIGQUEUE)printf("Catcher max number:\t%i\n",getLastVal());

    return 0;
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

