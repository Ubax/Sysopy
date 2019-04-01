#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "senderLib.h"

#ifndef SYSOPY_INITS_H
#define SYSOPY_INITS_H

int initSignals(void(*fun)(int, siginfo_t *, void *));
int initRTSignals(void(*fun)(int, siginfo_t *, void *));

#endif //SYSOPY_INITS_H
