#include <unistd.h>

#ifndef LOAD_H
#define LOAD_H

#define MAX_LOAD 100

typedef pid_t LoaderId;

struct Load{
  int weight;
  LoaderId loaderId;
};
#endif
