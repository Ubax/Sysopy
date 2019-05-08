#include <limits.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef LOAD_H
#define LOAD_H

#define MAX_LOAD 100
#define MAX_QUEUE_SIZE 100
#define CONVEYOR_BELT_FTOK_PATH getenv("HOME")
#define CONVEYOR_BELT_FTOK_PROJECT_NUM 1
#define CONVEYOR_BELT_FTOK                                                     \
  ftok(CONVEYOR_BELT_FTOK_PATH, CONVEYOR_BELT_FTOK_PROJECT_NUM);
#define CONVEYOR_BELT_SEM_MAX_ELEM 0
#define CONVEYOR_BELT_SEM_MAX_LOAD 1

typedef pid_t LoaderId;

struct Load {
  int weight;
  LoaderId loaderId;
};

struct ConveyorBeltQueue {
  int head;
  int tail;
  int size;
  int maxSize;
  struct Load array[MAX_QUEUE_SIZE];
};

void initConveyorBeltQueue(struct ConveyorBeltQueue *queue);
void push(struct ConveyorBeltQueue *queue, struct Load elem);
struct Load pop(struct ConveyorBeltQueue *queue);
int isEmpty(struct ConveyorBeltQueue *queue);
int isFull(struct ConveyorBeltQueue *queue);
void clear(struct ConveyorBeltQueue *queue);
#endif
