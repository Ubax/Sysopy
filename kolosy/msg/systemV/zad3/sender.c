#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY "."

struct MSGBUF {
  long mtype;
  char val[200];
};

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Not a suitable number of program parameters\n");
    return (1);
  }

  /******************************************************
  Utworz kolejke komunikatow systemu V "reprezentowana" przez KEY
  Wyslij do niej wartosc przekazana jako parametr wywolania programu
  Obowiazuja funkcje systemu V
  ******************************************************/
  key_t key = ftok(KEY, 1);
  if (key == -1) {
    perror("Ftok");
    return 1;
  }
  int msgId = msgget(key, IPC_CREAT | 0666);
  if (msgId < 0) {
    perror("SEn: MSGGET");
    return 1;
  }
  struct MSGBUF msg;
  msg.mtype = 1;
  strcpy(msg.val, argv[1]);
  if (msgsnd(msgId, &msg, sizeof(struct MSGBUF) - sizeof(long), 0) < 0) {
    perror("Sen: MSGSND");
    return 1;
  }
  return 0;
}
