#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef LOAD_H
#define LOAD_H

#define MAX_LOAD 100
#define CONVEYOR_BELT_FTOK ftok(getenv("home"), 1)

typedef pid_t LoaderId;

struct Load {
  int weight;
  LoaderId loaderId;
};
#endif
