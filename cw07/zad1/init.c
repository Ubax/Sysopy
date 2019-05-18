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
  if (argc < 2)
    MESSAGE_EXIT("Program expects at last 1 argument: number_of_loaders\t");
  numberOfLoaders = getArgAsInt(argv, 1);
  if (numberOfLoaders >= MAX_NUM_OF_LOADERS)
    if (argc - 2 < 2 * numberOfLoaders)
      MESSAGE_EXIT(
          "Program expects load and lifes for all loaders. Missing: %i",
          argc - 2 - 2 * numberOfLoaders);
  int i = 0;
  for (; i < MAX_NUM_OF_LOADERS; i++)
    children[i] = 0;
  for (i = 0; i < numberOfLoaders; i++) {
    children[i] = fork();
    if (children[i] == 0) {
      if (strcmp(argv[2 + 2 * i], "0") == 0)
        execl("./loader", "loader", argv[2 + 2 * i], NULL);
      else
        execl("./loader", "loader", argv[2 + 2 * i], argv[3 + 2 * i], NULL);
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
  for (i = 0; i < numberOfLoaders; i++) {
    int status;
    wait(&status);
  }
}
