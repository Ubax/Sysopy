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
int cyc = 0;
int done = 0;

sem_t *semaphoreMaxElem = NULL, *semaphorePriority = NULL, *semaphoreSet = NULL;
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
  if (conveyorBelt->maxWeight < packageLoad)
    MESSAGE_EXIT("Package is too heavy for the belt");
  while ((numberOfCycles > 0 || numberOfCycles == -2) &&
         conveyorBelt->truckExists) {
    double attempt = getCurrentTime();
    takeSem(semaphoreMaxElem);
    cyc = 0;
    while (!done && conveyorBelt->truckExists && cyc < HUNGER_LEVEL) {
      takeSem(semaphorePriority);
      if (push(semaphoreSet, conveyorBelt,
               (struct Load){packageLoad, getpid(), attempt})) {
        done = 1;
        placed();
      }
      releaseSem(semaphorePriority);
      waiting();
      cyc++;
    }
    if (!done) {
      takeSem(semaphorePriority);
      waiting();
      while (!push(semaphoreSet, conveyorBelt,
                   (struct Load){packageLoad, getpid(), attempt}) &&
             conveyorBelt->truckExists) {
      }
      releaseSem(semaphorePriority);
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
  sharedMemoryId = shm_open(CONVEYOR_BELT_MEM_PATH, O_RDWR, 0666);
  if (sharedMemoryId == -1)
    ERROR_EXIT("Creating shared memory");
  if (ftruncate(sharedMemoryId, sizeof(struct ConveyorBeltQueue)))
    ERROR_EXIT("Ftruncate");

  conveyorBelt = mmap(NULL, sizeof(struct ConveyorBeltQueue),
                      PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryId, 0);
  if (conveyorBelt == (void *)(-1))
    ERROR_EXIT("Attaching memory");

  semaphoreMaxElem = sem_open(CONVEYOR_BELT_SEM_MAX_ELEM, O_RDWR);
  if (semaphoreMaxElem == SEM_FAILED)
    ERROR_EXIT("Creating semaphore max elem");

  semaphorePriority = sem_open(CONVEYOR_BELT_SEM_PRIORITY, O_RDWR);
  if (semaphorePriority == SEM_FAILED)
    ERROR_EXIT("Creating semaphore max elem");

  semaphoreSet = sem_open(CONVEYOR_BELT_SEM_SET, O_RDWR);
  if (semaphoreSet == SEM_FAILED)
    ERROR_EXIT("Creating semaphore max elem");
}

void cleanExit() {
  munmap(conveyorBelt, sizeof(struct ConveyorBeltQueue));

  if (semaphoreMaxElem != NULL) {
    sem_close(semaphoreMaxElem);
  }
  if (semaphorePriority != NULL) {
    sem_close(semaphorePriority);
  }
  if (semaphoreSet != NULL) {
    sem_close(semaphoreSet);
  }
}
