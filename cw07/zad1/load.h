#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#ifndef LOAD_H
#define LOAD_H

#define MAX_LOAD 100

typedef pid_t LoaderId;

struct Load{
  int weight;
  LoaderId loaderId;
};
#endif
