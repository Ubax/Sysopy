#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define KEY "."

struct MSGBUF {
  long mtype;
  char val[200];
};

int main() {
  sleep(1);
  int val = 0;

  /**********************************
  Otworz kolejke systemu V "reprezentowana" przez KEY
  **********************************/
  key_t key = ftok(KEY, 1);
  if (key == -1) {
    perror("Ftok");
    return 1;
  }
  int msgId = msgget(key, 0);
  if (msgId < 0) {
    perror("Rec: MSGGET");
    return 1;
  }
  struct MSGBUF msg;

  // while(1)
  // {
  /**********************************
  Odczytaj zapisane w kolejce wartosci i przypisz je do zmiennej val
  obowiazuja funkcje systemu V
  ************************************/
  if (msgrcv(msgId, &msg, sizeof(struct MSGBUF) - sizeof(long), 0, 0) < 0) {
    perror("MSGRCV");
    return 1;
  }
  val = atoi(msg.val);
  printf("%d square is: %d\n", val, val * val);

  // }

  /*******************************
  posprzataj
  ********************************/
  msgctl(msgId, IPC_RMID, 0);

  return 0;
}
