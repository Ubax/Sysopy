#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "argsProcessor.h"
#include "senderLib.h"
#include "catcherLib.h"

int main(int argc, char **argv) {
    enum TYPE type;
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

    if(compareArg(argv, 3, "kill"))type = KILL;
    else if(compareArg(argv, 3, "sigqueue"))type = SIGQUEUE;
    else if(compareArg(argv, 3, "sigrt"))type = SIGRT;
    else{
        printf("Unknown sending method\n");
        return 1;
    }

    sender(catcherPID, type, numberOfSignals);
    int numberOfReceivedSignals = receive();
    printf("Received signals:\t%i\nSent signals:\t%ld\n", numberOfReceivedSignals, numberOfSignals);

    return 0;
}

