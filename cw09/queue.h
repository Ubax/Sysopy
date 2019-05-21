#ifndef QUEUE_H
#define QUEUE_H

struct Car {};

struct Passenger {};

struct Queue {
  int size;
  int maxSize;
  int head;
  int tail;
  void **data;
};

struct Queue createNewQueue(int maxSize);
void clearQueue(struct Queue *queue);
void push(struct Queue *queue, void *element);
void *pop(struct Queue *queue);
#endif
