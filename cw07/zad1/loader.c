#include "load.h"
#include "sysopy.h"

#define INFO(msg, ...)                                                         \
  {                                                                            \
    printf("[%f :: %i] ", getCurrentTime(), getpid());                         \
    printf(msg, ##__VA_ARGS__);                                                \
  }

void init();
void createConveyorBelt();
void cleanExit();
void waiting();
void placed();

int numberOfCycles = -2;
int packageLoad = 0;

int semaphoreId = -2;
int sharedMemoryId = -2;
int done = 0;
int cyc = 0;
struct ConveyorBeltQueue *conveyorBelt;

int main(int argc, char **argv) {
  if (argc < 2)
    MESSAGE_EXIT(
        "Program expects at last 1 argument: Package_load\t[number_of_cycles]");
  packageLoad = getArgAsInt(argv, 1);
  if (argc > 2)
    numberOfCycles = getArgAsInt(argv, 2);
  init();
  if (conveyorBelt->maxWeight < packageLoad)
    MESSAGE_EXIT("Package is too heavy for the belt");
  while ((numberOfCycles > 0 || numberOfCycles == -2) &&
         conveyorBelt->truckExists) {
    double attempt = getCurrentTime();
    takeSem(semaphoreId, CONVEYOR_BELT_SEM_MAX_ELEM);
    cyc = 0;
    while (!done && conveyorBelt->truckExists && cyc < HUNGER_LEVEL) {
      takeSem(semaphoreId, CONVEYOR_BELT_SEM_PRIORITY);
      if (push(semaphoreId, conveyorBelt,
               (struct Load){packageLoad, getpid(), attempt})) {
        done = 1;
        placed();
      }
      releaseSem(semaphoreId, CONVEYOR_BELT_SEM_PRIORITY);
      waiting();
      cyc++;
    }
    if (!done) {
      takeSem(semaphoreId, CONVEYOR_BELT_SEM_PRIORITY);
      while (!push(semaphoreId, conveyorBelt,
                   (struct Load){packageLoad, getpid(), attempt})) {
        waiting();
      }
      releaseSem(semaphoreId, CONVEYOR_BELT_SEM_PRIORITY);
      placed();
    }
    done = 0;
    if (numberOfCycles != -2)
      numberOfCycles--;
  }
  INFO("All work done for today. I'm going home")
  return 0;
}

void waiting() {
  INFO("Waiting for belt to free\nFree weigth: %i\tFree space: %i\n",
       conveyorBelt->maxWeight - conveyorBelt->weight,
       getSemState(semaphoreId, CONVEYOR_BELT_SEM_MAX_ELEM));
}
void placed() {
  INFO("Placed load on belt. Weight: %i\tFree weigth: %i\tFree space: "
       "%i\n",
       packageLoad, conveyorBelt->maxWeight - conveyorBelt->weight,
       getSemState(semaphoreId, CONVEYOR_BELT_SEM_MAX_ELEM));
}

void init() {
  // printf("------------\n| PID: %i\n------------\n", getpid());
  createConveyorBelt();
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
}

void createConveyorBelt() {
  key_t key = CONVEYOR_BELT_FTOK;
  if (key == -1)
    ERROR_EXIT("Getting key");
  sharedMemoryId = shmget(key, sizeof(struct ConveyorBeltQueue) + 10, 0);
  if (sharedMemoryId == -1)
    ERROR_EXIT("Creating shared memory");

  conveyorBelt = shmat(sharedMemoryId, 0, 0);
  if (conveyorBelt == (void *)(-1))
    ERROR_EXIT("Attaching memory");

  semaphoreId = semget(key, 0, 0);
  if (semaphoreId == -1)
    ERROR_EXIT("Creating semaphore");
}

void cleanExit() { shmdt(conveyorBelt); }
