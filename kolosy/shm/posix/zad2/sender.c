#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SHM_NAME "/kol_shm"
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
  int shmId = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
  if (shmId == -1) {
    perror("sen: shm_open");
    return 1;
  }
  if (ftruncate(shmId, 1024) == -1) {
    perror("ftruncate");
    return 1;
  }
  char *memory = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shmId, 0);
  strcpy(memory, argv[1]);
  munmap(memory, 1024);
  return 0;
}
