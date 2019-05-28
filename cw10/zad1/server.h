#define MAX_CLIENTS_NUMBER
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef SERVER_H
#define SERVER_H

struct CLIENT {
  char *name;
  int socketFD;
  struct sockaddr_in address;
  socklen_t socketLength;
};

#endif
