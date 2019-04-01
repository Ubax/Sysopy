#include "inits.h"

int initSignals(void(*fun)(int, siginfo_t *, void *)) {
    sigset_t signals;

    if (sigfillset(&signals) == -1) {
        perror("Signal Init");
        exit(1);
    }

    sigdelset(&signals, SIGUSR1);
    sigdelset(&signals, SIGUSR2);

    if (sigprocmask(SIG_BLOCK, &signals, NULL) != 0) {
        perror("Signal Init");
        exit(1);
    }

    if (sigemptyset(&signals) == -1) {
        perror("Signal Init");
        exit(1);
    }

    struct sigaction usr_action;
    usr_action.sa_sigaction = fun;
    usr_action.sa_mask = signals;
    usr_action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &usr_action, NULL) == -1 || sigaction(SIGUSR2, &usr_action, NULL) == -1) {
        perror("Signal Init");
        exit(1);
    }
    return 0;
}

int initRTSignals(void(*fun)(int, siginfo_t *, void *)) {
    sigset_t signals;

    if (sigfillset(&signals) == -1) {
        perror("Signal Init");
        exit(1);
    }

    sigdelset(&signals, SIG_REAL_USR1);
    sigdelset(&signals, SIG_REAL_USR2);

    if (sigprocmask(SIG_BLOCK, &signals, NULL) != 0) {
        perror("Signal Init");
        exit(1);
    }

    if (sigemptyset(&signals) == -1) {
        perror("Signal Init");
        exit(1);
    }

    struct sigaction usr_action;
    usr_action.sa_sigaction = fun;
    usr_action.sa_mask = signals;
    usr_action.sa_flags = SA_SIGINFO;

    if (sigaction(SIG_REAL_USR1, &usr_action, NULL) == -1 || sigaction(SIG_REAL_USR2, &usr_action, NULL) == -1) {
        perror("Signal Init");
        exit(1);
    }

    return 0;
}