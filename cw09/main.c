#include "queue.h"
#include "sysopy.h"
#include <pthread.h>

#define INFO(msg, ...)                                                         \
  {                                                                            \
    printf("[%f :: %ld] ", getCurrentTime(), pthread_self());                  \
    printf(msg, ##__VA_ARGS__);                                                \
  }

void *passenger(void *args);
void *car(void *args);
void start();
void cleanExit();

pthread_t *cars;
pthread_t *passengers;

struct Queue loadQueue;
struct Queue endPlatformQueue;
struct Car *currentCar = NULL;

int numberOfCars = 0;
int numberOfPassengers = 0;
int carCapacity = 0;
int numberOfRides = 0;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t end_platform_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t begin_of_ride_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_ready_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_empty_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t end_platform_change_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t all_passengers_created_cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char **argv) {
  int i;
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
  if (argc < 5) {
    MESSAGE_EXIT("Program expects 4 arguments: "
                 "number of passengers\t"
                 "number of cars\t"
                 "car capacity\t"
                 "number of rides");
  }
  numberOfPassengers = getArgAsInt(argv, 1);
  numberOfCars = getArgAsInt(argv, 2);
  carCapacity = getArgAsInt(argv, 3);
  numberOfRides = getArgAsInt(argv, 4);

  passengers = malloc(sizeof(pthread_t) * numberOfPassengers);
  if (passengers == NULL)
    ERROR_EXIT("Allocating threads");
  cars = malloc(sizeof(pthread_t) * numberOfCars);
  if (cars == NULL)
    ERROR_EXIT("Allocating threads");
  loadQueue = createNewQueue(numberOfCars);
  endPlatformQueue = createNewQueue(numberOfCars);

  printf("Creating passengers threads...\n");

  for (i = 0; i < numberOfPassengers; i++) {
    int *num = malloc(sizeof(int));
    *num = i;
    pthread_create(&passengers[i], NULL, passenger, num);
  }

  pthread_mutex_lock(&queue_mutex);
  pthread_cond_wait(&all_passengers_created_cond, &queue_mutex);
  pthread_mutex_unlock(&queue_mutex);

  printf("Creating cars threads...\n");

  for (i = 0; i < numberOfCars; i++) {
    pthread_create(&cars[i], NULL, car, NULL);
  }

  pthread_mutex_lock(&queue_mutex);
  while (loadQueue.size > 0) {
    pthread_cond_wait(&queue_empty_cond, &queue_mutex);
  }
  pthread_mutex_unlock(&queue_mutex);

  printf("Joining cars threads...\n");

  for (i = 0; i < numberOfPassengers; i++) {
    pthread_join(passengers[i], NULL);
  }

  printf("Joining passengers threads...\n");

  for (i = 0; i < numberOfCars; i++) {
    pthread_join(cars[i], NULL);
  }
  return 0;
}

void cleanExit() {
  if (cars != NULL)
    free(cars);
  if (passengers != NULL)
    free(passengers);
}

/***** MUST BE IN QUEUE MUTEX SECTION *******/
void start(struct Car *car) {
  car->canGetIn = 0;
  pthread_cond_broadcast(car->changedNumberOfPassengers);
  currentCar = NULL;
  INFO("Pressed start\n");
}

void *passenger(void *num) {
  pthread_mutex_lock(&queue_mutex);
  if (*(int *)num == numberOfPassengers - 1)
    pthread_cond_broadcast(&all_passengers_created_cond);
  pthread_cond_wait(&queue_ready_cond, &queue_mutex);
  pthread_mutex_unlock(&queue_mutex);

  pthread_mutex_lock(&queue_mutex);
  while (loadQueue.size > 0) {
    if (currentCar == NULL) {
      currentCar = pop(&loadQueue);
    }
    if (currentCar->canGetIn && sem_trywait(currentCar->semaphore) != -1) {
      struct Car *seat = currentCar;
      int sval;
      sem_getvalue(seat->semaphore, &sval);
      INFO("Walking into car %ld \tfree seats: %i/%i\n", *seat->thread_id, sval,
           carCapacity);
      if (sval == 0)
        start(seat);
      pthread_cond_wait(seat->endOfRide, &queue_mutex);
      sem_post(seat->semaphore);
      sem_getvalue(seat->semaphore, &sval);
      INFO("Walking outof car %ld \tfree seats: %i/%i\n", *seat->thread_id,
           sval, carCapacity);
      if (sval == carCapacity) {
        pthread_cond_broadcast(seat->empty);
        seat->canGetIn = 1;
      }
    } else {
      // printf("No free place\n");
    }
    pthread_mutex_unlock(&queue_mutex);
    pthread_mutex_lock(&queue_mutex);
  }
  free(num);
  INFO("It was so much fun. See you soon\n");
  pthread_mutex_unlock(&queue_mutex);
  return (void *)0;
}

struct Car *initThisCar() {
  struct Car *thisCar = malloc(sizeof(struct Car));
  if (thisCar == NULL)
    ERROR_EXIT("Couldn't initizalize memory for car");
  thisCar->thread_id = malloc(sizeof(pthread_t));
  thisCar->semaphore = malloc(sizeof(sem_t));
  thisCar->changedNumberOfPassengers = malloc(sizeof(pthread_cond_t));
  thisCar->endOfRide = malloc(sizeof(pthread_cond_t));
  thisCar->empty = malloc(sizeof(pthread_cond_t));
  *thisCar->thread_id = pthread_self();
  thisCar->canGetIn = 1;
  pthread_cond_init(thisCar->changedNumberOfPassengers, NULL);
  pthread_cond_init(thisCar->endOfRide, NULL);
  sem_init(thisCar->semaphore, 0, carCapacity);
  return thisCar;
}

void loadPassengers(struct Car *thisCar) {
  int sval;
  sem_getvalue(thisCar->semaphore, &sval);
  while (sval > 0) {
    INFO("Waiting for passengers\n");
    pthread_cond_wait(thisCar->changedNumberOfPassengers, &queue_mutex);
    sem_getvalue(thisCar->semaphore, &sval);
  }
}

void clearThisCar(struct Car *thisCar) {
  sem_destroy(thisCar->semaphore);
  free(thisCar->semaphore);
  free(thisCar->thread_id);
  free(thisCar->changedNumberOfPassengers);
  free(thisCar->endOfRide);
  free(thisCar->empty);
  free(thisCar);
}

void *car(void *args) {
  pthread_mutex_lock(&queue_mutex);

  struct Car *thisCar = initThisCar();

  push(&loadQueue, thisCar);
  if (loadQueue.size == loadQueue.maxSize) {
    pthread_cond_broadcast(&queue_ready_cond);
  }

  pthread_mutex_unlock(&queue_mutex);

  int i;
  for (i = 0; i < numberOfRides; i++) {
    pthread_mutex_lock(&queue_mutex);

    loadPassengers(thisCar);

    INFO("Closing doors\n");
    INFO("Begin of ride\n");
    if (i < numberOfRides - 1)
      push(&loadQueue, thisCar);
    push(&endPlatformQueue, thisCar);
    pthread_mutex_unlock(&queue_mutex);

    int time = rand() % 10;
    usleep(time * 1000);
    pthread_mutex_lock(&queue_mutex);
    INFO("End of ride\n");
    while (*head(&endPlatformQueue)->thread_id != pthread_self()) {
      pthread_cond_wait(&end_platform_change_cond, &queue_mutex);
    }
    pop(&endPlatformQueue);
    pthread_cond_broadcast(&end_platform_change_cond);
    INFO("Opening doors\n");
    pthread_cond_broadcast(thisCar->endOfRide);
    pthread_cond_wait(thisCar->empty, &queue_mutex);
    pthread_mutex_unlock(&queue_mutex);
  }

  INFO("Enough rides for today. Going to workshop\n");

  pthread_mutex_lock(&queue_mutex);
  clearThisCar(thisCar);
  pthread_mutex_unlock(&queue_mutex);
  return (void *)0;
}
