#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define UNIX_PATH_MAX 108
#define SOCK_PATH "sock_path"

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Not a suitable number of program parameters\n");
    return (1);
  }
  sleep(1);

  /*********************************************
  Utworz socket domeny unixowej typu datagramowego
  Utworz strukture adresowa ustawiajac adres/sciezke komunikacji na SOCK_PATH
  Polacz sie korzystajac ze zdefiniowanych socketu i struktury adresowej
  ***********************************************/
  int fd = -1;
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("creating socket");
    return 1;
  }
  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  snprintf(addr.sun_path, UNIX_PATH_MAX, "%s", SOCK_PATH);
  if (connect(fd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("Connecting");
    return 1;
  }
  char buff[60];
  int to_send = sprintf(buff, "%s", argv[1]);

  if (write(fd, buff, to_send + 1) == -1) {
    perror("Error sending msg to server");
  }

  /*****************************
  posprzataj po sockecie
  ********************************/
  unlink(SOCK_PATH);
  shutdown(fd, SHUT_RD);
  close(fd);
  return 0;
}
