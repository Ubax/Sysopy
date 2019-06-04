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

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Not a suitable number of program parameters\n");
    return (1);
  }

  /******************************************************
  Otworz kolejke POSIX "reprezentowana" przez MSG_NAME
  Wyslij do niej wartosc przekazana jako parametr wywolania programu
  Obowiazuja funkcje POSIX
  ******************************************************/
  struct mq_attr queue_attr;
  queue_attr.mq_maxmsg = 2;
  queue_attr.mq_msgsize = 2048;
  mqd_t queue =
      mq_open(MSG_NAME, O_WRONLY | O_CREAT | O_EXCL, 0666, &queue_attr);
  if (queue == -1) {
    perror("SEN: MQ_OPEN");
    return 1;
  }
  struct MSGBUF buf;
  strcpy(buf.val, argv[1]);
  if (mq_send(queue, (char *)&buf, sizeof(struct MSGBUF), 1) == -1) {
    perror("SEN: M_SEND");
    return 1;
  }
  sleep(2);
  return 0;
}
