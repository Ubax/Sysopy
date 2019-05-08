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

int numberOfCycles = -2;
int packageLoad = 0;

int semaphoreId = -2;
int sharedMemoryId = -2;
struct ConveyorBeltQueue *conveyorBelt;

int main(int argc, char **argv) {
  if (argc < 2)
    MESSAGE_EXIT(
        "Program expects at last 1 argument: Package_load\t[number_of_cycles]");
  packageLoad = getArgAsInt(argv, 1);
  if (argc > 2)
    numberOfCycles = getArgAsInt(argv, 2);
  init();
  while (numberOfCycles > 0 || numberOfCycles == -2) {
    double attempt = getCurrentTime();
    takeConvSem(semaphoreId, packageLoad);
    takeSetSem(semaphoreId);
    push(conveyorBelt, (struct Load){packageLoad, getpid(), attempt});
    INFO("Placed load on belt. Weight: %i\tFree weigth: %i\tFree space: %i\n",
         packageLoad, getSemState(semaphoreId, CONVEYOR_BELT_SEM_MAX_LOAD),
         getSemState(semaphoreId, CONVEYOR_BELT_SEM_MAX_ELEM));
    releaseSetSem(semaphoreId);
    if (numberOfCycles != -2)
      numberOfCycles--;
  }
  INFO("All work done for today. I'm going home")
  return 0;
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
