#include "queue.h"
#include "sysopy.h"

struct Queue createNewQueue(int maxSize) {
  struct Queue queue;
  queue.data = malloc(sizeof(void *) * maxSize);
  queue.maxSize = 0;
  queue.size = 0;
  int i;
  for (i = 0; i < maxSize; i++)
    queue.data[i] = NULL;
  return queue;
}
void clearQueue(struct Queue *queue) {
  if (queue->data == NULL)
    return;
  free(queue->data);
  queue->data = NULL;
  queue->maxSize = -1;
  queue->size = -1;
}
void push(struct Queue *queue, void *element) {
  if (queue->data[queue->tail] != NULL)
    ERROR_EXIT("Queue not empty");
  queue->data[queue->tail] = element;
  if (queue->tail >= queue->maxSize)
    queue->tail = 0;
  else
    queue->tail++;
}
void *pop(struct Queue *queue) {
  void *el = queue->data[queue->head];
  if (queue->head >= queue->maxSize)
    queue->head = 0;
  else
    queue->head++;
  return el;
}
