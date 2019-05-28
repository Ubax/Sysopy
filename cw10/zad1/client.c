#include "client.h"
#include "sysopy.h"

void cleanExit();

char *name;
char *address;
enum CONNECTION_TYPE connection_type = UNIX;

int main(int argc, char **argv) {
  if (atexit(cleanExit) == -1)
    MESSAGE_EXIT("Registering atexit failed");
  if (argc < 3) {
    MESSAGE_EXIT("Program expects 3 arguments: "
                 "name\t"
                 "connection type [NETWORK/UNIX]\t"
                 "server address");
  }
  name = argv[1];
  if (compareArg(argv, 2, "NETWORK"))
    connection_type = NETWORK;
  address = argv[3];
  return 0;
}

void cleanExit() {}
