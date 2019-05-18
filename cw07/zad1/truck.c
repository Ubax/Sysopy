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
int semaphoreMaxElem = -1, semaphoreSet = -1, semaphoreWrite = -1,
    semaphoreOnBelt = -1;
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
  sharedMemoryId = shmget(key, sizeof(struct ConveyorBeltQueue) + 10,
                          IPC_CREAT | IPC_EXCL | 0666);
  if (sharedMemoryId == -1)
    ERROR_EXIT("Creating shared memory");

  conveyorBelt = shmat(sharedMemoryId, 0, 0);
  if (conveyorBelt == (void *)(-1))
    ERROR_EXIT("Attaching memory");
  conveyorBelt->weight = 0;
  conveyorBelt->maxWeight = maxSummedWeightOfLoad;

  semaphoreSet = semget(keySet, 1, IPC_CREAT | IPC_EXCL | 0666);
  if (semaphoreSet == -1)
    ERROR_EXIT("Creating semaphoreSet");
  semaphoreMaxElem = semget(keyMaxElem, 1, IPC_CREAT | IPC_EXCL | 0666);
  if (semaphoreMaxElem == -1)
    ERROR_EXIT("Creating semaphoreMaxElem");
  semaphoreWrite = semget(keyWrite, 1, IPC_CREAT | IPC_EXCL | 0666);
  if (semaphoreWrite == -1)
    ERROR_EXIT("Creating semaphoreWrite");
  semaphoreOnBelt = semget(keyOnBelt, 1, IPC_CREAT | IPC_EXCL | 0666);
  if (semaphoreOnBelt == -1)
    ERROR_EXIT("Creating semaphoreOnBelt");

  setSemValue(semaphoreMaxElem, maxNumberOfLoads);
  setSemValue(semaphoreSet, 1);
  setSemValue(semaphoreWrite, 1);
  setSemValue(semaphoreOnBelt, 0);
}

void cleanExit() {
  conveyorBelt->truckExists = 0;
  clear(conveyorBelt);
  shmdt(conveyorBelt);
  if (sharedMemoryId >= 0) {
    shmctl(sharedMemoryId, IPC_RMID, NULL);
  }
  if (semaphoreMaxElem >= 0) {
    semctl(semaphoreMaxElem, 0, IPC_RMID);
  }
  if (semaphoreSet >= 0) {
    semctl(semaphoreSet, 0, IPC_RMID);
  }
  if (semaphoreWrite >= 0) {
    semctl(semaphoreWrite, 0, IPC_RMID);
  }
  if (semaphoreOnBelt >= 0) {
    semctl(semaphoreOnBelt, 0, IPC_RMID);
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
