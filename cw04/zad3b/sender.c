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

    struct sigaction actUsr1;
    sigemptyset(&actUsr1.sa_mask);
    actUsr1.sa_flags = SA_SIGINFO;
    actUsr1.sa_sigaction = &on_sigusr1;

    if (sigaction(SIGUSR1, &actUsr1, NULL) != 0)return -1;
    if (sigaction(SIG_REAL_USR1, &actUsr1, NULL) != 0)return -1;

    if(sender(catcherPID, type, numberOfSignals)){
        perror("sender");
        return 1;
    }

    return 0;
}

int sender(pid_t pid, enum TYPE type, size_t numberOfSignals) {
    size_t i=0;
    for(;i<numberOfSignals;i++){
        int ret = send(pid, type, SIG_SIGUSR1, i);
        if(ret!=0)return ret;
        waiting=1;
        while(waiting){sleep(1);}
    }
    return send(pid, type, SIG_SIGUSR2, i);
}

void on_sigusr1(int sig, siginfo_t *info, void *ucontext){
    printf("Received answer\n");
    waiting=0;
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

