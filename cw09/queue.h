#include <semaphore.h>

#ifndef QUEUE_H
#define QUEUE_H

struct Car {
  pthread_t *thread_id;
  sem_t *semaphore;
  int canGetIn;
  int inRide;
  pthread_cond_t *changedNumberOfPassengers;
  pthread_cond_t *endOfRide;
  pthread_cond_t *empty;
};

struct Passenger {
  pthread_t *thread_id;
};

struct Queue {
  int size;
  int maxSize;
  int head;
  int tail;
  struct Car **data;
};

struct Queue createNewQueue(int maxSize);
void clearQueue(struct Queue *queue);
void push(struct Queue *queue, struct Car *element);
struct Car *pop(struct Queue *queue);
struct Car *head(struct Queue *queue);
#endif
