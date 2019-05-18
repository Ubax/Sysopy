#include "sysopy.h"
#include <limits.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef LOAD_H
#define LOAD_H

#define MAX_LOAD 100
#define MAX_QUEUE_SIZE 100
#define CONVEYOR_BELT_FTOK_PATH getenv("HOME")
#define CONVEYOR_BELT_FTOK_PROJECT_NUM 1

#define CONVEYOR_BELT_SEM_MAX_ELEM 2
#define CONVEYOR_BELT_SEM_WRITE 3
#define CONVEYOR_BELT_SEM_ON_BELT 4
#define CONVEYOR_BELT_SEM_SET 5

typedef pid_t LoaderId;

struct Load {
  int weight;
  LoaderId loaderId;
  double timeOfAttempt;
  double timeOfPlacement;
};

struct ConveyorBeltQueue {
  int head;
  int tail;
  int size;
  int maxSize;
  int weight;
  int maxWeight;
  int truckExists;
  struct Load array[MAX_QUEUE_SIZE];
};

void initConveyorBeltQueue(struct ConveyorBeltQueue *queue);
int push(struct ConveyorBeltQueue *queue, struct Load elem);
struct Load pop(struct ConveyorBeltQueue *queue);
int isEmpty(struct ConveyorBeltQueue *queue);
int isFull(struct ConveyorBeltQueue *queue);
void clear(struct ConveyorBeltQueue *queue);
void takeSem(int semid);
void releaseSem(int semid);
int getSemState(int semid);
void setSemValue(int semid, int value);
int canPush(struct ConveyorBeltQueue *queue, struct Load elem);

#endif
