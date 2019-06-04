#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define UNIX_PATH_MAX 108
#define SOCK_PATH "sock_path"

int main() {
  int fd = -1;

  /*********************************************
  Utworz socket domeny unixowej typu datagramowego
  Utworz strukture adresowa ustawiajac adres/sciezke komunikacji na SOCK_PATH
  Zbinduj socket z adresem/sciezka SOCK_PATH
  **********************************************/
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("creating socket");
    return 1;
  }
  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  snprintf(addr.sun_path, UNIX_PATH_MAX, "%s", SOCK_PATH);
  if (bind(fd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("Binding");
    return 1;
  }

  char buf[20];
  if (read(fd, buf, 20) == -1)
    perror("Error receiving message");
  int val = atoi(buf);
  printf("%d square is: %d\n", val, val * val);

  /***************************
  Posprzataj po sockecie
  ****************************/
  unlink(SOCK_PATH);
  shutdown(fd, SHUT_RD);
  close(fd);
  return 0;
}
