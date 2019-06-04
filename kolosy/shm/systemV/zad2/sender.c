#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SHM_NAME "./sender.c"
#define MAX_SIZE 1024

int main(int argc, char **argv) {

  if (argc != 2) {
    printf("Not a suitable number of program parameters\n");
    return (1);
  }

  /***************************************
  Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
  zapisz tam wartosc przekazana jako parametr wywolania programu
  posprzataj
  *****************************************/

  key_t key = ftok(SHM_NAME, 2);
  if (key == -1) {
    perror("sen: ftok");
    return 1;
  }
  int shmId = shmget(key, MAX_SIZE, IPC_CREAT | IPC_EXCL | 0666);
  if (shmId == -1) {
    perror("sen: shmget");
    return 1;
  }
  char *memory = shmat(shmId, NULL, 0);
  if (memory == (void *)-1) {
    perror("sen: shmat");
    return 1;
  }
  strcpy(memory, argv[1]);
  shmdt(memory);
  return 0;
}
