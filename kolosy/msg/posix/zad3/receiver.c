#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MSG_NAME "/queue"

struct MSGBUF {
  char val[50];
};

int main() {
  sleep(1);
  int val = 0;

  /**********************************
  Otworz kolejke POSIX "reprezentowana" przez MSG_NAME
  **********************************/
  mqd_t queue = mq_open(MSG_NAME, O_RDONLY);
  if (queue == -1) {
    perror("rec: MQ_OPEN");
    return 1;
  }
  struct MSGBUF buf;

  /**********************************
  Odczytaj zapisane w kolejce wartosci i przypisz je do zmiennej val
  obowiazuja funkcje POSIX
  ************************************/
  if (mq_receive(queue, (char *)&buf, sizeof(struct MSGBUF), NULL) == -1) {
    perror("rec: mq_receive");
    return 1;
  }
  val = atoi(buf.val);
  printf("%d square is: %d\n", val, val * val);

  /*******************************
  posprzataj
  ********************************/
  mq_close(queue);
  mq_unlink(MSG_NAME);
  return 0;
}
