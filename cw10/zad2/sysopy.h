
//#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/un.h>
#include <unistd.h>

#ifndef SYSOPY_H
#define SYSOPY_H

#define ERROR_EXIT(msg)                                                        \
  {                                                                            \
    perror(msg);                                                               \
    exit(1);                                                                   \
  }
#define MESSAGE_EXIT(msg, ...)                                                 \
  {                                                                            \
    printf(msg, ##__VA_ARGS__);                                                \
    exit(1);                                                                   \
  }

int compareArg(char **argv, int id, const char *value);
int compare(char *template, const char *value);
int getArgAsInt(char **argv, int id);
size_t getArgAsSizeT(char **argv, int id);
void toUpper(char *str);
double getCurrentTime();

#endif // SYSOPY_H
