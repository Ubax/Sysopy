#include "load.h"
#include "sysopy.h"

void takeSetSem(int semid) {
  struct sembuf buf;
  buf.sem_num = CONVEYOR_BELT_SEM_SET;
  buf.sem_op = -1;
  buf.sem_flg = 0;
  if (semop(semid, &buf, 1) == -1)
    ERROR_EXIT("Taking set semaphore");
}
void releaseSetSem(int semid) {
  struct sembuf buf;
  buf.sem_num = CONVEYOR_BELT_SEM_SET;
  buf.sem_op = 1;
  buf.sem_flg = 0;
  if (semop(semid, &buf, 1) == -1)
    ERROR_EXIT("Releasing set semaphore");
}

void takeConvSem(int semid, int weight) {
  struct sembuf buf;
  buf.sem_num = CONVEYOR_BELT_SEM_MAX_ELEM;
  buf.sem_op = -1;
  buf.sem_flg = 0;
  if (semop(semid, &buf, 1) == -1)
    ERROR_EXIT("Taking conveyor elem semaphore");
  buf.sem_num = CONVEYOR_BELT_SEM_MAX_LOAD;
  buf.sem_op = -weight;
  if (semop(semid, &buf, 1) == -1)
    ERROR_EXIT("Taking conveyor weight semaphore");
}

void releaseConvSem(int semid, int weight) {
  struct sembuf buf;
  buf.sem_num = CONVEYOR_BELT_SEM_MAX_ELEM;
  buf.sem_op = 1;
  buf.sem_flg = 0;
  if (semop(semid, &buf, 1) == -1)
    ERROR_EXIT("Releasing conveyor elem semaphore");
  buf.sem_num = CONVEYOR_BELT_SEM_MAX_LOAD;
  buf.sem_op = weight;
  if (semop(semid, &buf, 1) == -1)
    ERROR_EXIT("Releasing conveyor weight semaphore");
}

/******
RETURN VALUE: oldIndex
******/
int incQueueIndex(struct ConveyorBeltQueue *queue, int *index) {
  int oldIndex = *index;
  if (oldIndex + 1 >= queue->maxSize) {
    *index = 0;
  } else
    (*index)++;
  return oldIndex;
}

void initConveyorBeltQueue(struct ConveyorBeltQueue *queue) {
  if (queue == NULL)
    MESSAGE_EXIT("null queue");
  queue->head = 0;
  queue->tail = 0;
  queue->maxSize = MAX_QUEUE_SIZE;
  queue->size = 0;
}
void push(struct ConveyorBeltQueue *queue, struct Load elem) {
  if (queue == NULL || queue->maxSize == 0)
    MESSAGE_EXIT("null queue");
  if (isFull(queue))
    MESSAGE_EXIT("full queue");
  queue->array[incQueueIndex(queue, &queue->tail)] = elem;
  queue->size++;
}
struct Load pop(struct ConveyorBeltQueue *queue) {
  if (queue == NULL || queue->size == 0)
    MESSAGE_EXIT("null queue");
  if (isEmpty(queue))
    MESSAGE_EXIT("full queue");

  queue->size--;
  return queue->array[incQueueIndex(queue, &queue->head)];
}
int isEmpty(struct ConveyorBeltQueue *queue) { return queue->size == 0; }
int isFull(struct ConveyorBeltQueue *queue) {
  return queue->size == queue->maxSize;
}
void clear(struct ConveyorBeltQueue *queue) {
  queue->head = 0;
  queue->tail = 0;
  queue->maxSize = 0;
  queue->size = 0;
}
