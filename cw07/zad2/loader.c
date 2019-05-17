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
int done = 0;

sem_t *semaphoreMaxElem = NULL, *semaphoreSet = NULL, *semaphoreWrite = NULL,
      *semaphoreOnBelt = NULL;
int sharedMemoryId = -2;
struct ConveyorBeltQueue *conveyorBelt;

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

  semaphoreWrite = sem_open(CONVEYOR_BELT_SEM_WRITE, O_RDWR);
  if (semaphoreWrite == SEM_FAILED)
    ERROR_EXIT("Creating semaphore write");

  semaphoreSet = sem_open(CONVEYOR_BELT_SEM_SET, O_RDWR);
  if (semaphoreSet == SEM_FAILED)
    ERROR_EXIT("Creating semaphore set");

  semaphoreOnBelt = sem_open(CONVEYOR_BELT_SEM_ON_BELT, O_RDWR);
  if (semaphoreOnBelt == SEM_FAILED)
    ERROR_EXIT("Creating semaphore on belt");
}

void cleanExit() {
  INFO("All work done for today. I'm going home\n")
  munmap(conveyorBelt, sizeof(struct ConveyorBeltQueue));

  if (semaphoreOnBelt != NULL) {
    sem_close(semaphoreOnBelt);
  }
  if (semaphoreMaxElem != NULL) {
    sem_close(semaphoreMaxElem);
  }
  if (semaphoreWrite != NULL) {
    sem_close(semaphoreWrite);
  }
  if (semaphoreSet != NULL) {
    sem_close(semaphoreSet);
  }
}
