#include "load.h"
#include "sysopy.h"

void init();
void createConveyorBelt();
void cleanExit();

int truckMaxLoad, maxNumberOfLoads, maxSummedWeightOfLoad;
int occupiedSpace;
int sharedMemoryId = -2;
int semaphoreId = -2;

int main(int argc, char **argv) {
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
  truckMaxLoad = getArgAsInt(argv, 1);
  maxNumberOfLoads = getArgAsInt(argv, 2);
  maxSummedWeightOfLoad = getArgAsInt(argv, 3);
  printf("%i, %i, %i\n", truckMaxLoad, maxNumberOfLoads, maxSummedWeightOfLoad);
  init();
  return 0;
}

void init() {
  occupiedSpace = 0;
  createConveyorBelt();
}

void createConveyorBelt() {
  sharedMemoryId =
      shmget(CONVEYOR_BELT_FTOK, maxNumberOfLoads * sizeof(struct Load),
             IPC_CREAT | IPC_EXCL | 0666);
  if (sharedMemoryId == -1)
    ERROR_EXIT("Creating shared memory");

  semaphoreId = semget(CONVEYOR_BELT_FTOK, 2, IPC_CREAT | IPC_EXCL | 0666);
  if (semaphoreId == -1)
    ERROR_EXIT("Creating semaphore");
}

void cleanExit() {
  if (sharedMemoryId >= 0) {
    shmctl(sharedMemoryId, IPC_RMID, NULL);
  }
  if (semaphoreId >= 0) {
    semctl(semaphoreId, 0, IPC_RMID);
  }
}
