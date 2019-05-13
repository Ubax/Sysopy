#include "sysopy.h"
#include <sys/wait.h>

#define MAX_NUM_OF_LOADERS 1000

void cleanExit();
void signalHandler(int signo);

char *truckMaxLoad, *maxNumberOfLoads, *maxSummedWeightOfLoad;
int numberOfLoaders = 0;
pid_t children[MAX_NUM_OF_LOADERS + 1];
int main(int argc, char **argv) {
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
  signal(SIGINT, signalHandler);
  if (argc < 5)
    MESSAGE_EXIT("Program expects at last 4 arguments: "
                 "truck_max_load\tmax_number_of_loads\tmax_summed_weight_of_"
                 "load\tnumber_of_loaders\t");
  truckMaxLoad = argv[1];
  maxNumberOfLoads = argv[2];
  maxSummedWeightOfLoad = argv[3];
  numberOfLoaders = getArgAsInt(argv, 4);
  if (numberOfLoaders >= MAX_NUM_OF_LOADERS)
    if (argc - 5 < numberOfLoaders)
      MESSAGE_EXIT("Program expects load for all loaders. Missing: %i",
                   argc - 5 - numberOfLoaders);
  int i = 0;
  for (; i < MAX_NUM_OF_LOADERS; i++)
    children[i] = 0;
  children[0] = fork();
  if (children[0] == 0) {
    execl("./truck", "truck", truckMaxLoad, maxNumberOfLoads,
          maxSummedWeightOfLoad, NULL);
  }
  for (i = 0; i < numberOfLoaders; i++) {
    children[i + 1] = fork();
    if (children[i + 1] == 0) {
      execl("./loader", "loader", argv[5 + i], NULL);
    }
  }

  return 0;
}

void signalHandler(int signo) {
  if (signo == SIGINT) {
    int i;
    for (i = 0; i < numberOfLoaders + 1; i++) {
      kill(children[i], SIGINT);
    }
    exit(0);
  }
}

void cleanExit() {
  int i;
  for (i = 0; i < numberOfLoaders + 1; i++) {
    int status;
    wait(&status);
  }
}
