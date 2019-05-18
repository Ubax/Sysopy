#include "load.h"
#include "sysopy.h"

#define INFO(msg, ...)                                                         \
  {                                                                            \
    takeSem(semaphoreWrite);                                                   \
    printf("[%f :: %i] ", getCurrentTime(), getpid());                         \
    printf(msg, ##__VA_ARGS__);                                                \
    releaseSem(semaphoreWrite);                                                \
  }

void init();
void createConveyorBelt();
void cleanExit();
void waiting();
void placed();

int numberOfCycles = -2;
int packageLoad = 0;

int sharedMemoryId = -2;
int done = 0;
int cyc = 0;
struct ConveyorBeltQueue *conveyorBelt;

int semaphoreMaxElem = -1, semaphoreSet = -1, semaphoreWrite = -1,
    semaphoreOnBelt = -1;

struct Load package;

int main(int argc, char **argv) {
  if (argc < 2)
    MESSAGE_EXIT(
        "Program expects at last 1 argument: Package_load\t[number_of_cycles]");
  packageLoad = getArgAsInt(argv, 1);
  if (argc > 2)
    numberOfCycles = getArgAsInt(argv, 2);

  init();

  INFO("Init = Load: %i\tCycles: %i\n", packageLoad, numberOfCycles);

  if (conveyorBelt->maxWeight < packageLoad)
    MESSAGE_EXIT("Package is too heavy for the belt\n");

  while ((numberOfCycles > 0 || numberOfCycles == -2) &&
         conveyorBelt->truckExists) {
    package = (struct Load){packageLoad, getpid(), getCurrentTime(), 0};
    while (!done && conveyorBelt->truckExists) {
      takeSem(semaphoreSet);
      if (canPush(conveyorBelt, package)) {
        takeSem(semaphoreMaxElem);
        package.timeOfPlacement = getCurrentTime();
        push(conveyorBelt, package);
        releaseSem(semaphoreOnBelt);
        done = 1;
        placed();
      }
      if (!done)
        waiting();
      releaseSem(semaphoreSet);
      usleep(2);
    }
    done = 0;
    if (numberOfCycles != -2)
      numberOfCycles--;
  }
  return 0;
}

void waiting() {
  INFO("Waiting for belt to free. Free weigth: %i\tFree space: %i\n",
       conveyorBelt->maxWeight - conveyorBelt->weight,
       getSemState(semaphoreMaxElem));
}
void placed() {
  INFO("Placed load on belt. Weight: %i\tFree weigth: %i\tFree space: "
       "%i\n",
       packageLoad, conveyorBelt->maxWeight - conveyorBelt->weight,
       getSemState(semaphoreMaxElem));
}

void init() {
  // printf("------------\n| PID: %i\n------------\n", getpid());
  createConveyorBelt();
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
}

void createConveyorBelt() {
  key_t key = ftok(CONVEYOR_BELT_FTOK_PATH, CONVEYOR_BELT_FTOK_PROJECT_NUM);
  if (key == -1)
    ERROR_EXIT("Getting key");
  key_t keyMaxElem = ftok(CONVEYOR_BELT_FTOK_PATH, CONVEYOR_BELT_SEM_MAX_ELEM);
  if (keyMaxElem == -1)
    ERROR_EXIT("Getting keyMaxElem");
  key_t keySet = ftok(CONVEYOR_BELT_FTOK_PATH, CONVEYOR_BELT_SEM_SET);
  if (keySet == -1)
    ERROR_EXIT("Getting keySet");
  key_t keyWrite = ftok(CONVEYOR_BELT_FTOK_PATH, CONVEYOR_BELT_SEM_WRITE);
  if (keyWrite == -1)
    ERROR_EXIT("Getting keyWrite");
  key_t keyOnBelt = ftok(CONVEYOR_BELT_FTOK_PATH, CONVEYOR_BELT_SEM_ON_BELT);
  if (keyOnBelt == -1)
    ERROR_EXIT("Getting keyOnBelt");
  sharedMemoryId = shmget(key, sizeof(struct ConveyorBeltQueue) + 10, 0);
  if (sharedMemoryId == -1)
    ERROR_EXIT("Creating shared memory");

  conveyorBelt = shmat(sharedMemoryId, 0, 0);
  if (conveyorBelt == (void *)(-1))
    ERROR_EXIT("Attaching memory");

  semaphoreSet = semget(keySet, 0, 0);
  if (semaphoreSet == -1)
    ERROR_EXIT("Creating semaphoreSet");
  semaphoreMaxElem = semget(keyMaxElem, 0, 0);
  if (semaphoreMaxElem == -1)
    ERROR_EXIT("Creating semaphoreMaxElem");
  semaphoreWrite = semget(keyWrite, 0, 0);
  if (semaphoreWrite == -1)
    ERROR_EXIT("Creating semaphoreWrite");
  semaphoreOnBelt = semget(keyOnBelt, 0, 0);
  if (semaphoreOnBelt == -1)
    ERROR_EXIT("Creating semaphoreOnBelt");
}

void cleanExit() { shmdt(conveyorBelt); }
