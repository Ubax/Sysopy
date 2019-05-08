#include "load.h"
#include "sysopy.h"

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
    // printf("Trying to get semaphores\n");
    takeConvSem(semaphoreId, packageLoad);
    // printf("Got conv semaphore\n");
    takeSetSem(semaphoreId);
    // printf("Got set semaphore\n");
    push(conveyorBelt, (struct Load){packageLoad, getpid(), attempt});
    printf("[%f] Placed load on belt. Weight: %i\tPid: %i\n", getCurrentTime(),
           packageLoad, getpid());
    releaseSetSem(semaphoreId);
    // printf("Released semaphores\n");
    if (numberOfCycles != -2)
      numberOfCycles--;
  }
  printf("Finished\n");
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
