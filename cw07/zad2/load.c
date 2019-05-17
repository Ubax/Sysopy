#include "load.h"
#include "sysopy.h"

int getSemState(sem_t *semid) {
  int ret;
  if (sem_getvalue(semid, &ret) == -1)
    ERROR_EXIT("Getting semaphore value");
  return ret;
}

void takeSem(sem_t *sem) {
  if (sem_wait(sem) == -1)
    ERROR_EXIT("Taking semaphore");
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

int canPush(struct ConveyorBeltQueue *queue, struct Load elem) {
  if (queue == NULL || queue->maxSize == 0)
    MESSAGE_EXIT("null queue");
  if (isFull(queue))
    return 0;
  if (queue->maxWeight < queue->weight + elem.weight) {
    return 0;
  }
  return 1;
}

int push(struct ConveyorBeltQueue *queue, struct Load elem) {
  if (queue == NULL || queue->maxSize == 0)
    MESSAGE_EXIT("null queue");
  if (isFull(queue))
    MESSAGE_EXIT("full queue");
  queue->array[incQueueIndex(queue, &queue->tail)] = elem;
  queue->size++;
  queue->weight += elem.weight;
  return 1;
}
struct Load pop(struct ConveyorBeltQueue *queue) {
  if (queue == NULL || queue->size == 0)
    MESSAGE_EXIT("null queue");
  if (isEmpty(queue))
    MESSAGE_EXIT("full queue");
  queue->size--;
  struct Load ret = queue->array[incQueueIndex(queue, &queue->head)];
  queue->weight -= ret.weight;
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
