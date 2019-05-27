#include "queue.h"
#include "sysopy.h"

struct Queue createNewQueue(int maxSize) {
  struct Queue queue;
  queue.data = malloc(sizeof(void *) * maxSize);
  queue.maxSize = maxSize;
  queue.size = 0;
  queue.head = 0;
  queue.tail = 0;
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
void push(struct Queue *queue, struct Car *element) {
  if (queue->data[queue->tail] != NULL)
    MESSAGE_EXIT("Queue not empty %i %i", queue->tail, queue->maxSize);
  queue->data[queue->tail] = element;
  if (queue->tail + 1 >= queue->maxSize)
    queue->tail = 0;
  else
    queue->tail++;
  queue->size++;
}

struct Car *head(struct Queue *queue) {
  return queue->data[queue->head];
}

struct Car *pop(struct Queue *queue) {
  void *el = head(queue);
  queue->data[queue->head] = NULL;
  if (queue->head + 1 >= queue->maxSize)
    queue->head = 0;
  else
    queue->head++;
  queue->size--;
  return el;
}
