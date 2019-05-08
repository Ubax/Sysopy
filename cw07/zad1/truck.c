#include "load.h"
#include "sysopy.h"

void init();
void createConveyorBelt();
void cleanExit();

int truckMaxLoad, maxNumberOfLoads, maxSummedWeightOfLoad;
int occupiedSpace;
int sharedMemoryId = -2;
int semaphoreId = -2;
struct ConveyorBeltQueue *conveyorBelt;

int main(int argc, char **argv) {
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
  truckMaxLoad = getArgAsInt(argv, 1);
  maxNumberOfLoads = getArgAsInt(argv, 2);
  maxSummedWeightOfLoad = getArgAsInt(argv, 3);
  if (maxNumberOfLoads > MAX_QUEUE_SIZE)
    MESSAGE_EXIT("Too big conveyor belt");
  init();
  return 0;
}

void init() {
  occupiedSpace = 0;
  createConveyorBelt();
  initConveyorBeltQueue(conveyorBelt);
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

  semaphoreId = semget(key, 2, IPC_CREAT | IPC_EXCL | 0666);
  if (semaphoreId == -1)
    ERROR_EXIT("Creating semaphore");

  semctl(semaphoreId, CONVEYOR_BELT_SEM_MAX_ELEM, SETVAL, maxNumberOfLoads);
  semctl(semaphoreId, CONVEYOR_BELT_SEM_MAX_LOAD, SETVAL,
         maxSummedWeightOfLoad);
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
