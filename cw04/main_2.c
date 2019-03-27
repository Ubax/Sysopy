#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void signalTSTP(int);

void signalINT(int);

void makeAlive();

int isProcessDead = 0;
int waitingForAction = 0;

pid_t childPid;

int main(int argc, char **argv) {
    struct sigaction act;
    act.sa_handler = signalTSTP;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    makeAlive();

    sigaction(SIGTSTP, &act, NULL);
    signal(SIGINT, &signalINT);

    while (1) {
        sleep(1);
    }
    return 0;
}

void makeAlive(){
    isProcessDead = 0;
    childPid = fork();
    if (childPid == 0) {
        execl("./dataPrinter.sh", "./dataPrinter.sh", NULL);
    }
}

void signalTSTP(int signalno) {
    if (!isProcessDead) {
        isProcessDead = 1;
        kill(childPid, SIGKILL);
        int staus;
        wait(&staus);
    }
    else if (isProcessDead) {
        makeAlive();
    }
    if(!waitingForAction)printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
    waitingForAction=!waitingForAction;
}

void signalINT(int signalno) {
    if(!isProcessDead)kill(childPid, SIGKILL);
    exit(EXIT_SUCCESS);
}

