#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define FILE_NAME "common.txt"
#define SEM_NAME "./main.c"

void semwait(int semId) {
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = -1;
  buf.sem_flg = 0;
  semop(semId, &buf, 1);
}

void sempost(int semId) {
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = 1;
  buf.sem_flg = 0;
  semop(semId, &buf, 1);
}

int main(int argc, char **args) {

  if (argc != 4) {
    printf("Not a suitable number of program parameters\n");
    return (1);
  }

  key_t key = ftok(SEM_NAME, 1);
  int semId = semget(key, 1, IPC_CREAT | IPC_EXCL);
  semctl(semId, 0, SETVAL, 1);

  int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  int parentLoopCounter = atoi(args[1]);
  int childLoopCounter = atoi(args[2]);

  char buf[50];
  pid_t childPid;
  int max_sleep_time = atoi(args[3]);

  if ((childPid = fork()) != 0) {
    int status = 0;
    srand((unsigned)time(0));

    while (parentLoopCounter--) {
      int s = rand() % max_sleep_time + 1;
      sleep(s);

      /*****************************************
      sekcja krytyczna zabezpiecz dostep semaforem
      **********************************************/

      semwait(semId);

      sprintf(buf, "Wpis rodzica. Petla %d. Spalem %d\n", parentLoopCounter, s);
      write(fd, buf, strlen(buf));
      write(1, buf, strlen(buf));

      /*********************************
      Koniec sekcji krytycznej
      **********************************/
      sempost(semId);
    }
    waitpid(childPid, &status, 0);
  } else {

    srand((unsigned)time(0));
    while (childLoopCounter--) {

      int s = rand() % max_sleep_time + 1;
      sleep(s);

      /*****************************************
      sekcja krytyczna zabezpiecz dostep semaforem
      **********************************************/

      semwait(semId);

      sprintf(buf, "Wpis dziecka. Petla %d. Spalem %d\n", childLoopCounter, s);
      write(fd, buf, strlen(buf));
      write(1, buf, strlen(buf));
      sempost(semId);

      /*********************************
      Koniec sekcji krytycznej
      **********************************/
    }
    _exit(0);
  }

  /*****************************
  posprzataj semafor
  ******************************/
  semctl(semId, 0, IPC_RMID);
  close(fd);
  return 0;
}
