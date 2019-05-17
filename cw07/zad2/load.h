#include "sysopy.h"
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef LOAD_H
#define LOAD_H

#define MAX_LOAD 100
#define MAX_QUEUE_SIZE 100
#define CONVEYOR_BELT_MEM_PATH "/conveyorbeltmem"
#define CONVEYOR_BELT_SEM_MAX_ELEM "/conveyorbeltelem"
#define CONVEYOR_BELT_SEM_WRITE "/conveyorbeltwrite"
#define CONVEYOR_BELT_SEM_ON_BELT "/conveyorbeltonbelt"
#define CONVEYOR_BELT_SEM_SET "/conveyorbeltset"

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
int canPush(struct ConveyorBeltQueue *queue, struct Load elem);
struct Load pop(struct ConveyorBeltQueue *queue);
int isEmpty(struct ConveyorBeltQueue *queue);
int isFull(struct ConveyorBeltQueue *queue);
void clear(struct ConveyorBeltQueue *queue);
void takeSem(sem_t *sem_set);
int takeSemNonblock(sem_t *sem_set);
void releaseSem(sem_t *sem_set);
int getSemState(sem_t *semid);

#endif
