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
int numberOfWorkingCars = 0;
int randomPassengerId = -1;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t end_platform_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t begin_of_ride_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_ready_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_empty_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t platform_change_cond = PTHREAD_COND_INITIALIZER;
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
  numberOfWorkingCars = numberOfCars = getArgAsInt(argv, 2);
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

  pthread_mutex_lock(&queue_mutex);
  printf("Creating cars threads...\n");

  for (i = 0; i < numberOfCars; i++) {
    int *index = malloc(sizeof(int));
    *index = i;
    pthread_create(&cars[i], NULL, car, index);
  }
  pthread_cond_wait(&queue_ready_cond, &queue_mutex);

  printf("Creating passengers threads...\n");

  for (i = 0; i < numberOfPassengers; i++) {
    int *num = malloc(sizeof(int));
    *num = i;
    pthread_create(&passengers[i], NULL, passenger, num);
  }
  pthread_mutex_unlock(&queue_mutex);

  // pthread_mutex_lock(&queue_mutex);
  // pthread_cond_wait(&all_passengers_created_cond, &queue_mutex);
  // pthread_mutex_unlock(&queue_mutex);

  pthread_mutex_lock(&queue_mutex);
  while (numberOfWorkingCars > 0) {
    pthread_cond_wait(&queue_empty_cond, &queue_mutex);
  }
  pthread_mutex_unlock(&queue_mutex);

  for (i = 0; i < numberOfPassengers; i++) {
    pthread_join(passengers[i], NULL);
  }

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
  INFO("Pressed start in car %ld\n", *car->thread_id);
  car->shouldStart = 1;
  pthread_cond_broadcast(car->changedNumberOfPassengers);
}

void *passenger(void *num) {
  int numOfPassRides = 0;
  pthread_mutex_lock(&queue_mutex);
  if (*(int *)num == numberOfPassengers - 1)
    pthread_cond_signal(&all_passengers_created_cond);
  pthread_mutex_unlock(&queue_mutex);

  while (numberOfWorkingCars > 0) {
    pthread_mutex_lock(&queue_mutex);
    if (currentCar != NULL && currentCar->canGetIn &&
        sem_trywait(currentCar->semaphore) != -1) {
      numOfPassRides++;

      struct Car *seat = currentCar;
      int sval;
      sem_getvalue(seat->semaphore, &sval);
      INFO("Walking into cart %ld \tfree seats: %i/%i\n", *seat->thread_id,
           sval, carCapacity);
      int id = sval;
      if (sval == 0) {
        randomPassengerId = rand() % carCapacity;
        pthread_cond_signal(seat->beginRide);
        pthread_mutex_unlock(&queue_mutex);
        pthread_mutex_lock(&queue_mutex);
      } else {
        pthread_cond_wait(seat->beginRide, &queue_mutex);
        pthread_cond_signal(seat->beginRide);
        pthread_mutex_unlock(&queue_mutex);
        pthread_mutex_lock(&queue_mutex);
      }
      if (randomPassengerId == id)
        start(seat);
      pthread_cond_wait(seat->endOfRide, &queue_mutex);
      sem_post(seat->semaphore);
      sem_getvalue(seat->semaphore, &sval);
      INFO("Walking outof cart %ld \tfree seats: %i/%i\n", *seat->thread_id,
           sval, carCapacity);
      if (sval == carCapacity) {
        pthread_cond_signal(seat->empty);
        seat->canGetIn = 1;
      }
    } else {
      // printf("No free place\n");
    }
    pthread_mutex_unlock(&queue_mutex);
  }
  pthread_mutex_lock(&queue_mutex);
  free(num);
  INFO("\x1b[32;1mIt was so much fun. I rode %i times. See you soon\x1b[0m\n",
       numOfPassRides);
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
  thisCar->beginRide = malloc(sizeof(pthread_cond_t));
  thisCar->empty = malloc(sizeof(pthread_cond_t));
  *thisCar->thread_id = pthread_self();
  thisCar->canGetIn = 1;
  pthread_cond_init(thisCar->changedNumberOfPassengers, NULL);
  pthread_cond_init(thisCar->endOfRide, NULL);
  pthread_cond_init(thisCar->beginRide, NULL);
  sem_init(thisCar->semaphore, 0, carCapacity);
  thisCar->shouldStart = 0;
  return thisCar;
}

void loadPassengers(struct Car *thisCar) {
  while (!thisCar->shouldStart) {
    pthread_cond_wait(thisCar->changedNumberOfPassengers, &queue_mutex);
  }
  thisCar->shouldStart = 0;
}

void clearThisCar(struct Car *thisCar) {
  sem_destroy(thisCar->semaphore);
  free(thisCar->semaphore);
  free(thisCar->thread_id);
  free(thisCar->changedNumberOfPassengers);
  free(thisCar->endOfRide);
  free(thisCar->beginRide);
  free(thisCar->empty);
  free(thisCar);
}

void *car(void *args) {
  int arrayId = *((int *)args);
  free(args);
  struct Car *thisCar = initThisCar();

  pthread_mutex_lock(&queue_mutex);
  push(&loadQueue, thisCar);
  if (arrayId == numberOfCars - 1) {
    pthread_cond_signal(&queue_ready_cond);
  }
  pthread_mutex_unlock(&queue_mutex);

  pthread_mutex_lock(&queue_mutex);
  int i = 0;
  while (*head(&loadQueue)->thread_id != pthread_self()) {
    pthread_cond_signal(&platform_change_cond);
    pthread_cond_wait(&platform_change_cond, &queue_mutex);
  }
  while (currentCar != NULL) {
    pthread_cond_signal(&platform_change_cond);
    pthread_cond_wait(&platform_change_cond, &queue_mutex);
  }

  currentCar = pop(&loadQueue);

  pthread_mutex_unlock(&queue_mutex);
  for (; i < numberOfRides; i++) {
    pthread_mutex_lock(&queue_mutex);

    loadPassengers(thisCar);

    currentCar = NULL;
    INFO("\x1b[34;1mClosing doors\x1b[0m\n");
    INFO("\x1b[34;1mBegin of ride\x1b[0m\n");

    push(&loadQueue, thisCar);

    // push(&endPlatformQueue, thisCar);
    pthread_cond_signal(&platform_change_cond);
    pthread_mutex_unlock(&queue_mutex);

    int time = rand() % 10;
    usleep(time * 1000);
    pthread_mutex_lock(&queue_mutex);
    // while (*head(&endPlatformQueue)->thread_id != pthread_self()) {
    //   pthread_cond_signal(&platform_change_cond);
    //   pthread_cond_wait(&platform_change_cond, &queue_mutex);
    // }
    // INFO("\x1b[34;1mEnd of ride\x1b[0m\n");
    // pop(&endPlatformQueue);
    while (*head(&loadQueue)->thread_id != pthread_self()) {
      pthread_cond_signal(&platform_change_cond);
      pthread_cond_wait(&platform_change_cond, &queue_mutex);
    }
    while (currentCar != NULL) {
      pthread_cond_signal(&platform_change_cond);
      pthread_cond_wait(&platform_change_cond, &queue_mutex);
    }
    INFO("\x1b[34;1mEnd of ride\x1b[0m\n");
    INFO("\x1b[34;1mOpening doors\x1b[0m\n");
    pthread_cond_broadcast(thisCar->endOfRide);
    pthread_cond_wait(thisCar->empty, &queue_mutex);
    pop(&loadQueue);
    if (i < numberOfRides - 1) {
      currentCar = thisCar;
    }
    pthread_cond_signal(&platform_change_cond);
    pthread_mutex_unlock(&queue_mutex);
  }

  pthread_mutex_lock(&queue_mutex);
  numberOfWorkingCars--;
  INFO("\x1b[32;1mEnough rides for today. Going to workshop\x1b[0m\n");
  pthread_cond_broadcast(&queue_empty_cond);
  pthread_mutex_unlock(&queue_mutex);

  clearThisCar(thisCar);
  return (void *)0;
}
