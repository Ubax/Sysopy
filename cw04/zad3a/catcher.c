#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void printWelcome();
void printGoodbye();

int receiving = 1;
int sending = 0;
int numberOfsignals = 0;
pid_t senderPID;

void sigUsr1(int signalno) {
    if(receiving)numberOfsignals++;
}

void sigUsr2(int sig, siginfo_t *info, void *ucontext) {
    receiving = 0;
    sending = 1;
    senderPID=info->si_pid;
}

int main(int argc, char **argv) {
    printWelcome();
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags=SA_SIGINFO;
    act.sa_sigaction=&sigUsr2;

    sigaction(SIGUSR2, &act, NULL);
    signal(SIGUSR1, &sigUsr1);
    while (receiving) {
        sleep(1);
    }
    int i=0;
    for(;i<numberOfsignals;i++){
        kill(senderPID, SIGUSR1);
    }
    kill(senderPID, SIGUSR2);
    printGoodbye();
    return 0;
}

void printWelcome() {
    printf("PID:\t%i\n", getpid());
}

void printGoodbye(){
    printf("Number of received signals:\t%i\n", numberOfsignals);
}