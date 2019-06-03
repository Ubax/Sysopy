#include "server.h"
#include "sysopy.h"

void cleanExit();
void initSockets();
void initEpoll();

int portNumber;
char *socketPath;
struct sockaddr_in serverIPAddress;
struct sockaddr_un serverUnixAddress;
int epollFD;
int socketIPFD, socketUnixFD;
struct CLIENT clients[MAX_CLIENTS_NUMBER];

int main(int argc, char **argv) {
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
  if (argc < 2) {
    MESSAGE_EXIT("Program expects 2 arguments: "
                 "TCP/UDP port number\t"
                 "UNIX socket path");
  }
  portNumber = getArgAsInt(argv, 1);
  if (portNumber < 1024 || portNumber > 65535)
    MESSAGE_EXIT("Port number out of range [1024, 65535]");
  if (strlen(argv[2]) > 90)
    MESSAGE_EXIT("Too long path name");
  socketPath = argv[2];

  initSockets();

  return 0;
}

void initSockets() {
  socketIPFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketIPFD < 0)
    ERROR_EXIT("Opening IP socket");
  socketUnixFD = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (socketUnixFD < 0)
    ERROR_EXIT("Opening unix socket");

  serverIPAddress.sin_family = AF_INET;
  serverIPAddress.sin_addr.s_addr = INADDR_ANY;
  serverIPAddress.sin_port = htons(portNumber);

  serverUnixAddress.sun_family = AF_LOCAL;
  sprintf(serverUnixAddress.sun_path, "%s", socketPath);

  if (bind(socketUnixFD, (struct sockaddr *)&serverUnixAddress,
           sizeof(serverUnixAddress)) < 0)
    ERROR_EXIT("Binding unix socket");
  if (listen(socketUnixFD, 32) == -1)
    ERROR_EXIT("Listeing on unix socket");
}

void initEpoll() {
  if ((epollFD = epoll_create1(0)) == -1)
    ERROR_EXIT("Creating epoll");
}

void cleanExit() {
  close(socketIPFD);
  close(socketUnixFD);
  unlink(socketPath);
}
