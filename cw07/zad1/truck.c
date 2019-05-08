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
int semaphoreId = -2;
struct ConveyorBeltQueue *conveyorBelt;

int main(int argc, char **argv) {
  signal(SIGINT, signalHandler);
  getDataFromArgs(argc, argv);
  init();
  while (1) {
    if (!isEmpty(conveyorBelt)) {
      struct Load ret = pop(conveyorBelt);
      if (ret.weight > truckMaxLoad - occupiedSpace) {
        INFO("Truck is full\n");
        emptyTruck();
      }
      occupiedSpace += ret.weight;
      INFO("Taken from belt. LoaderId: %i\n"
           "\tOccupied space in truck: %i\n"
           "\tPackage weight: %i\t"
           "\tDiff time: %f\n",
           ret.loaderId, occupiedSpace, ret.weight,
           (getCurrentTime() - ret.timeOfAttempt));
      releaseConvSem(semaphoreId, ret.weight);
    } else {
      INFO("Waiting for package\n");
    }
    sleep(1);
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
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
  emptyTruck();
}

void createConveyorBelt() {
  key_t key = CONVEYOR_BELT_FTOK;
  if (key == -1)
    ERROR_EXIT("Getting key");
  sharedMemoryId = shmget(key, sizeof(struct ConveyorBeltQueue) + 10,
                          IPC_CREAT | IPC_EXCL | 0666);
  if (sharedMemoryId == -1)
    ERROR_EXIT("Creating shared memory");

  conveyorBelt = shmat(sharedMemoryId, 0, 0);
  if (conveyorBelt == (void *)(-1))
    ERROR_EXIT("Attaching memory");

  semaphoreId = semget(key, 3, IPC_CREAT | IPC_EXCL | 0666);
  if (semaphoreId == -1)
    ERROR_EXIT("Creating semaphore");

  setSemValue(semaphoreId, CONVEYOR_BELT_SEM_MAX_ELEM, maxNumberOfLoads);
  setSemValue(semaphoreId, CONVEYOR_BELT_SEM_MAX_LOAD, maxSummedWeightOfLoad);
  setSemValue(semaphoreId, CONVEYOR_BELT_SEM_SET, 1);
}

void cleanExit() {
  clear(conveyorBelt);
  shmdt(conveyorBelt);
  if (sharedMemoryId >= 0) {
    shmctl(sharedMemoryId, IPC_RMID, NULL);
  }
  if (semaphoreId >= 0) {
    semctl(semaphoreId, 0, IPC_RMID);
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
