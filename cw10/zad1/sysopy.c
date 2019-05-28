#include "sysopy.h"

double getCurrentTime() {
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  return currentTime.tv_sec + currentTime.tv_usec * 1.0 / 1e6;
}

void toUpper(char *str) {
  size_t i = 0;
  for (; i < strlen(str); i++)
    str[i] = (char)toupper(str[i]);
}

int compareArg(char **argv, int id, const char *value) {
  char arg[strlen(argv[id]) + 1];
  char val[strlen(value) + 1];
  strcpy(arg, argv[id]);
  strcpy(val, value);
  toUpper(arg);
  toUpper(val);
  return strcmp(arg, val) == 0;
}

int compare(char *template, const char *value) {
  char arg[strlen(template) + 1];
  char val[strlen(value) + 1];
  strcpy(arg, template);
  strcpy(val, value);
  toUpper(arg);
  toUpper(val);
  return strcmp(arg, val) == 0;
}

int getArgAsInt(char **argv, int id) {
  int ret = (int)strtol(argv[id], NULL, 10);
  if (errno != 0)
    ERROR_EXIT("Getting int arg");
  return ret;
}

size_t getArgAsSizeT(char **argv, int id) {
  int ret = (int)strtol(argv[id], NULL, 10);
  if (ret < 1) {
    printf("Number of signals should be positive\n");
    return 1;
  }
  return (size_t)ret;
}
