#include "load.h"
#include "sysopy.h"

int getSemState(sem_t *semid) {
  int ret;
  if (sem_getvalue(semid, &ret) == -1)
    ERROR_EXIT("Getting semaphore value");
  return ret;
}

void takeSem(sem_t *sem, struct ConveyorBeltQueue *belt) {
  while (!takeSemNonblock(sem)) {
    if (!belt->truckExists)
      exit(0);
    if (errno != EAGAIN)
      ERROR_EXIT("Taking  semaphore");
  }
}

int takeSemNonblock(sem_t *sem) { return sem_trywait(sem) != -1; }

void releaseSem(sem_t *sem) {
  if (sem_post(sem) == -1)
    ERROR_EXIT("Releasing semaphore");
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
int push(sem_t *sem_set, struct ConveyorBeltQueue *queue, struct Load elem) {
  if (queue == NULL || queue->maxSize == 0)
    MESSAGE_EXIT("null queue");
  if (isFull(queue))
    MESSAGE_EXIT("full queue");
  takeSem(sem_set, queue);
  if (queue->maxWeight < queue->weight + elem.weight) {
    releaseSem(sem_set);
    return 0;
  }
  queue->array[incQueueIndex(queue, &queue->tail)] = elem;
  queue->size++;
  queue->weight += elem.weight;
  releaseSem(sem_set);
  return 1;
}
struct Load pop(sem_t *sem_set, struct ConveyorBeltQueue *queue) {
  if (queue == NULL || queue->size == 0)
    MESSAGE_EXIT("null queue");
  if (isEmpty(queue))
    MESSAGE_EXIT("full queue");
  takeSem(sem_set, queue);
  queue->size--;
  struct Load ret = queue->array[incQueueIndex(queue, &queue->head)];
  queue->weight -= ret.weight;
  releaseSem(sem_set);
  return ret;
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
