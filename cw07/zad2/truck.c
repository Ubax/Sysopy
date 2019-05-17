#include "load.h"
#include "sysopy.h"

#define INFO(msg, ...)                                                         \
  {                                                                            \
    printf("[%f] ", getCurrentTime());                                         \
    printf(msg, ##__VA_ARGS__);                                                \
  }

void init();
void createConveyorBelt();
void cleanExit();
void signalHandler(int signo);
void emptyTruck();
void getDataFromArgs(int argc, char **argv);

int truckMaxLoad, maxNumberOfLoads, maxSummedWeightOfLoad;
int occupiedSpace;
int sharedMemoryId = -2;
sem_t *semaphoreMaxElem = NULL, *semaphoreSet = NULL, *semaphoreWrite = NULL,
      *semaphoreOnBelt = NULL;
struct ConveyorBeltQueue *conveyorBelt;

int main(int argc, char **argv) {
  signal(SIGINT, signalHandler);
  getDataFromArgs(argc, argv);
  init();
  while (1) {
    if (isEmpty(conveyorBelt))
      INFO("Waiting for package\n");
    takeSem(semaphoreOnBelt);
    takeSem(semaphoreSet);
    struct Load ret = pop(conveyorBelt);
    if (ret.weight > truckMaxLoad - occupiedSpace) {
      INFO("Truck is full\n");

      emptyTruck();
    }
    occupiedSpace += ret.weight;
    INFO("Taken from belt. LoaderId: %i\n"
         "\tOccupied space in truck: %i\n"
         "\tPackage weight: %i\t"
         "\tDiff time: %f\tPlacement time: %f\n",
         ret.loaderId, occupiedSpace, ret.weight,
         (getCurrentTime() - ret.timeOfAttempt), ret.timeOfPlacement);
    releaseSem(semaphoreMaxElem);
    releaseSem(semaphoreSet);
  }
  return 0;
}

void getDataFromArgs(int argc, char **argv) {
  if (argc < 4)
    MESSAGE_EXIT(
        "Program expects at last 3 argument: "
        "truck_max_load\tmax_number_of_loads\tmax_summed_weight_of_load");
  truckMaxLoad = getArgAsInt(argv, 1);
  maxNumberOfLoads = getArgAsInt(argv, 2);
  maxSummedWeightOfLoad = getArgAsInt(argv, 3);
  if (maxNumberOfLoads > MAX_QUEUE_SIZE)
    MESSAGE_EXIT("Too big conveyor belt");
  printf("Truck max load: %i\nBelt max number of elements: %i\nBelt max load: "
         "%i\n",
         truckMaxLoad, maxNumberOfLoads, maxSummedWeightOfLoad);
}

void init() {
  createConveyorBelt();
  initConveyorBeltQueue(conveyorBelt);
  conveyorBelt->truckExists = 1;
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
  emptyTruck();
}

void createConveyorBelt() {
  sharedMemoryId =
      shm_open(CONVEYOR_BELT_MEM_PATH, O_CREAT | O_RDWR | O_EXCL, 0666);
  if (sharedMemoryId == -1)
    ERROR_EXIT("Creating shared memory");
  if (ftruncate(sharedMemoryId, sizeof(struct ConveyorBeltQueue)))
    ERROR_EXIT("Ftruncate");

  conveyorBelt = mmap(NULL, sizeof(struct ConveyorBeltQueue),
                      PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryId, 0);
  if (conveyorBelt == (void *)(-1))
    ERROR_EXIT("Attaching memory");
  conveyorBelt->weight = 0;
  conveyorBelt->maxWeight = maxSummedWeightOfLoad;

  semaphoreMaxElem =
      sem_open(CONVEYOR_BELT_SEM_MAX_ELEM, O_CREAT | O_RDWR | O_EXCL, 0666,
               maxNumberOfLoads);
  if (semaphoreMaxElem == SEM_FAILED)
    ERROR_EXIT("Creating semaphore max elem");

  semaphoreWrite =
      sem_open(CONVEYOR_BELT_SEM_WRITE, O_CREAT | O_RDWR | O_EXCL, 0666, 1);
  if (semaphoreWrite == SEM_FAILED)
    ERROR_EXIT("Creating write semaphore");

  semaphoreOnBelt =
      sem_open(CONVEYOR_BELT_SEM_ON_BELT, O_CREAT | O_RDWR | O_EXCL, 0666, 0);
  if (semaphoreOnBelt == SEM_FAILED)
    ERROR_EXIT("Creating on belt semaphore");

  semaphoreSet =
      sem_open(CONVEYOR_BELT_SEM_SET, O_CREAT | O_RDWR | O_EXCL, 0666, 1);
  if (semaphoreSet == SEM_FAILED)
    ERROR_EXIT("Creating semaphore max elem");
}

void cleanExit() {
  conveyorBelt->truckExists = 0;
  clear(conveyorBelt);
  munmap(conveyorBelt, sizeof(struct ConveyorBeltQueue));
  if (sharedMemoryId >= 0) {
    shm_unlink(CONVEYOR_BELT_MEM_PATH);
  }
  if (semaphoreMaxElem != NULL) {
    sem_close(semaphoreMaxElem);
    sem_unlink(CONVEYOR_BELT_SEM_MAX_ELEM);
  }
  if (semaphoreOnBelt != NULL) {
    sem_close(semaphoreOnBelt);
    sem_unlink(CONVEYOR_BELT_SEM_ON_BELT);
  }
  if (semaphoreWrite != NULL) {
    sem_close(semaphoreWrite);
    sem_unlink(CONVEYOR_BELT_SEM_WRITE);
  }
  if (semaphoreSet != NULL) {
    sem_close(semaphoreSet);
    sem_unlink(CONVEYOR_BELT_SEM_SET);
  }
}

void signalHandler(int signo) {
  if (signo == SIGINT) {
    printf("You killed me :'(\n");
    exit(0);
  }
}

void emptyTruck() {
  INFO("New truck came\n");
  occupiedSpace = 0;
}
