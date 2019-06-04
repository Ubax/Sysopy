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

  sleep(1);
  int val = 0;
  /*******************************************
  Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
  odczytaj zapisana tam wartosc i przypisz ja do zmiennej val
  posprzataj
  *********************************************/
  int shmId = shm_open(SHM_NAME, O_RDWR, 0);
  if (shmId == -1) {
    perror("rec: shm_open");
    return 1;
  }
  char *memory = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shmId, 0);
  val = atoi(memory);
  printf("%d square is: %d \n", val, val * val);
  munmap(memory, 1024);
  shm_unlink(SHM_NAME);
  return 0;
}
