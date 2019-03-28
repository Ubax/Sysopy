#include <signal.h>
#include "senderLib.h"

#ifndef SYSOPY_CATCHERLIB_H
#define SYSOPY_CATCHERLIB_H

#define SIG_REAL_USR1 SIGRTMIN+3
#define SIG_REAL_USR2 SIGRTMIN+4

int receive(enum TYPE type);
pid_t getSenderPid();
int getLastVal();

#endif //SYSOPY_CATCHERLIB_H
