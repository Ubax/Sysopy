#include <signal.h>

#ifndef SYSOPY_CATCHERLIB_H
#define SYSOPY_CATCHERLIB_H

#define SIG_REAL_USR1 SIGRTMIN+3
#define SIG_REAL_USR2 SIGRTMIN+4

int receive();
pid_t getSenderPid();
int isQueue();
int getLastIntValue();

#endif //SYSOPY_CATCHERLIB_H
