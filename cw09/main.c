#include "sysopy.h"
#include <pthread.h>

#define INFO(msg, ...)                                                         \
  {                                                                            \
    printf("[%f :: %ld] ", getCurrentTime(), pthread_self());                  \
    printf(msg, ##__VA_ARGS__);                                                \
  }

void *passenger(void *args);
void *car(void *args);
void cleanExit();

pthread_t *cars;
pthread_t *passengers;

int numberOfCars = 0;
int numberOfPassengers = 0;
int carCapacity = 0;
int numberOfRides = 0;

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

  for (i = 0; i < numberOfPassengers; i++) {
    pthread_create(&passengers[i], NULL, passenger, NULL);
  }

  for (i = 0; i < numberOfCars; i++) {
    pthread_create(&cars[i], NULL, car, NULL);
  }

  sleep(2);

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

void *passenger(void *args) {
  INFO("Passenger\n");
  return (void *)0;
}
void *car(void *args) {
  INFO("Car\n");
  return (void *)0;
}
