#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void signalTSTP(int);
void signalINT(int);

int waitingForAction = 0;

int main(int argc, char ** argv){
    struct sigaction act;
    act.sa_handler=signalTSTP;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;

    sigaction(SIGTSTP, &act, NULL);
    signal(SIGINT, &signalINT);

    while (1){
        if(!waitingForAction)system("date");
        sleep(1);
    }
    return 0;
}

void signalTSTP(int signalno){
    if(!waitingForAction)printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
    waitingForAction=!waitingForAction;
}

void signalINT(int signalno){
    printf("CTRL+C\n");
    exit(signalno);
}

