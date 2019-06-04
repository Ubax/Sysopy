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

  sleep(1);
  int val = 0;
  /*******************************************
  Utworz/otworz posixowy obszar pamieci wspolnej "reprezentowany" przez SHM_NAME
  odczytaj zapisana tam wartosc i przypisz ja do zmiennej val
  posprzataj
  *********************************************/

  key_t key = ftok(SHM_NAME, 2);
  if (key == -1) {
    perror("rec: ftok");
    return 1;
  }
  int shmId = shmget(key, MAX_SIZE, 0);
  if (shmId == -1) {
    perror("rec: shmget");
    return 1;
  }
  char *memory = shmat(shmId, NULL, 0);
  if (memory == (void *)-1) {
    perror("rec: shmat");
    return 1;
  }
  val = atoi(memory);
  shmdt(memory);
  shmctl(shmId, IPC_RMID, NULL);

  printf("%d square is: %d \n", val, val * val);
  return 0;
}
