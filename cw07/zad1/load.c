#include "load.h"
#include "sysopy.h"

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
