#include "server.h"
#include "sysopy.h"
__uint16_t port_number;

int unix_socket;
int web_socket;
int epoll;
char *unix_socket_path;

uint64_t id;

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
struct CLIENT clients[MAX_CLIENTS_NUMBER];

pthread_t pinger;
pthread_t commander;

void init();

void *commands_fun(void *arg);
void *ping_fun(void *arg);

void handle_reg(int);
void handle_call(int);

void send_msg(int sock, struct SOCKET_MSG msg);
void send_empty(int, enum SOCKET_MSG_TYPE);

void delete_client(int);
void delete_socket(int);

int client_by_name(char*);

struct SOCKET_MSG get_msg(int);
void delete_msg(struct SOCKET_MSG msg);

void signal_handler(int signo);
void cleanExit();

int main(int argc, char *argv[]) {
  if (atexit(cleanExit) == -1)
  MESSAGE_EXIT("Registering atexit failed");
  if (argc < 2) {
    MESSAGE_EXIT("Program expects 2 arguments: "
                 "TCP/UDP port number\t"
                 "UNIX socket path");
  }
  port_number = (__uint16_t )getArgAsInt(argv, 1);
  if (port_number < 1024 || port_number > 65535)
  MESSAGE_EXIT("Port number out of range [1024, 65535]");
  if (strlen(argv[2]) > UNIX_PATH_MAX)
  MESSAGE_EXIT("Too long path name");
  unix_socket_path = argv[2];

  init();

  struct epoll_event event;
  while (1) {
    if (epoll_wait(epoll, &event, 1, -1) == -1) ERROR_EXIT("");

    if (event.data.fd < 0)
      handle_reg(-event.data.fd);
    else
      handle_call(event.data.fd);
  }
}

void init()
{
  struct sockaddr_in web_addr;
  memset(&web_addr, 0, sizeof(struct sockaddr_in));
  web_addr.sin_family = AF_INET;
  web_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
  web_addr.sin_port = htons(port_number);

  if ((web_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    ERROR_EXIT("");

  if (bind(web_socket, (const struct sockaddr *) &web_addr, sizeof(web_addr)))
    ERROR_EXIT("");

  if (listen(web_socket, 64) == -1)
    ERROR_EXIT("");

  // init unix socket
  struct sockaddr_un un_addr;
  un_addr.sun_family = AF_UNIX;

  snprintf(un_addr.sun_path, UNIX_PATH_MAX, "%s", unix_socket_path);

  if ((unix_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    ERROR_EXIT("");

  if (bind(unix_socket, (const struct sockaddr *) &un_addr, sizeof(un_addr)))
    ERROR_EXIT("");

  if (listen(unix_socket, MAX_CLIENTS_NUMBER) == -1)
    ERROR_EXIT("");

  // init epoll
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLPRI;

  if ((epoll = epoll_create1(0)) == -1)
    ERROR_EXIT("");

  event.data.fd = -web_socket;
  if (epoll_ctl(epoll, EPOLL_CTL_ADD, web_socket, &event) == -1)
    ERROR_EXIT("");

  event.data.fd = -unix_socket;
  if (epoll_ctl(epoll, EPOLL_CTL_ADD, unix_socket, &event) == -1)
    ERROR_EXIT("");

  // start threads
  if (pthread_create(&commander, NULL, commands_fun, NULL) != 0)
    ERROR_EXIT("");
  if (pthread_detach(commander) != 0)
    ERROR_EXIT("");

  if (pthread_create(&pinger, NULL, ping_fun, NULL) != 0)
    ERROR_EXIT("");
  if (pthread_detach(pinger) != 0)
    ERROR_EXIT("");
}

void *commands_fun(void *params)
{
  char buffer[1024];
  while (1) {
    int min_i = MAX_CLIENTS_NUMBER;
    int min = 1000000;

    // read command
    scanf("%1023s", buffer);

    // open file
    FILE *file = fopen(buffer, "r");
    if (file == NULL) {
      perror("");
      continue;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char *file_buff = malloc(size + 1);
    if (file_buff == NULL) {
      perror("");
      continue;
    }

    file_buff[size] = '\0';

    if (fread(file_buff, 1, size, file) != size) {
      fprintf(stderr, "Could not read file\n");
      free(file_buff);
      continue;
    }

    fclose(file);

    // send request
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
      if (!clients[i].socketFD) continue;
      if (min > clients[i].working) {
        min_i = i;
        min = clients[i].working;
      }
    }

    if (min_i < MAX_CLIENTS_NUMBER) {
      struct SOCKET_MSG msg = { WORK, strlen(file_buff) + 1, 0, ++id, file_buff, NULL };
      printf("JOB %lu SEND TO %s\n", id, clients[min_i].name);
      send_msg(clients[min_i].socketFD, msg);
      clients[min_i].working++;
    } else {
      fprintf(stderr, "No clients connected\n");
    }
    pthread_mutex_unlock(&client_mutex);

    free(file_buff);
  }
}

void *ping_fun(void *params)
{
  while (1) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
      if (clients[i].socketFD == 0) continue;
      if (clients[i].inactive) {
        delete_client(i);
      } else {
        clients[i].inactive = 1;
        send_empty(clients[i].socketFD, PING);
      }
    }
    pthread_mutex_unlock(&client_mutex);
    sleep(10);
  }
}

void handle_reg(int sock)
{
  puts("Client registered");
  int client = accept(sock, NULL, NULL);
  if (client == -1) ERROR_EXIT("");
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLPRI;
  event.data.fd = client;

  if (epoll_ctl(epoll, EPOLL_CTL_ADD, client, &event) == -1)
    ERROR_EXIT("");
}

void handle_call(int sock)
{
  struct SOCKET_MSG msg = get_msg(sock);
  pthread_mutex_lock(&client_mutex);

  switch (msg.type) {
    case REGISTER: {
      enum SOCKET_MSG_TYPE reply = OK;
      int i;
      i = client_by_name(msg.name);
      if (i < MAX_CLIENTS_NUMBER)
        reply = NAME_TAKEN;

      for (i = 0; i < MAX_CLIENTS_NUMBER && clients[i].socketFD != 0; i++);

      if (i == MAX_CLIENTS_NUMBER)
        reply = FULL;

      if (reply != OK) {
        send_empty(sock, reply);
        delete_socket(sock);
        break;
      }

      clients[i].socketFD = sock;
      clients[i].name = malloc(msg.size + 1);
      if (clients[i].name == NULL) ERROR_EXIT("");
      strcpy(clients[i].name, msg.name);
      clients[i].working = 0;
      clients[i].inactive = 0;

      send_empty(sock, OK);
      break;
    }
    case UNREGISTER: {
      int i;
      for (i = 0; i < MAX_CLIENTS_NUMBER && strcmp(clients[i].name, msg.name) != 0; i++);
      if (i == MAX_CLIENTS_NUMBER) break;
      delete_client(i);
      break;
    }
    case WORK_DONE: {
      int i = client_by_name(msg.name);
      if (i < MAX_CLIENTS_NUMBER) {
        clients[i].inactive = 0;
        clients[i].working--;
      }
      printf("JOB %lu DONE BY %s:\n%s\n", msg.id, (char*) msg.name, (char*) msg.content);
      break;
    }
    case PONG: {
      int i = client_by_name(msg.name);
      if (i < MAX_CLIENTS_NUMBER)
        clients[i].inactive = 0;
    }
  }

  pthread_mutex_unlock(&client_mutex);

  delete_msg(msg);
}

void delete_client(int i)
{
  delete_socket(clients[i].socketFD);
  clients[i].socketFD = 0;
  clients[i].name = NULL;
  clients[i].working = 0;
  clients[i].inactive = 0;
}

int client_by_name(char *name)
{
  int i;
  for (i = 0; i < MAX_CLIENTS_NUMBER; i++) {
    if (clients[i].socketFD == 0) continue;
    if (strcmp(clients[i].name, name) == 0)
      break;
  }
  return i;
}

void delete_socket(int sock) {
  epoll_ctl(epoll, EPOLL_CTL_DEL, sock, NULL);
  shutdown(sock, SHUT_RDWR);
  close(sock);
}

void send_msg(int sock, struct SOCKET_MSG msg) {
  write(sock, &msg.type, sizeof(msg.type));
  write(sock, &msg.size, sizeof(msg.size));
  write(sock, &msg.name_size, sizeof(msg.name_size));
  write(sock, &msg.id, sizeof(msg.id));
  if (msg.size > 0) write(sock, msg.content, msg.size);
  if (msg.name_size > 0) write(sock, msg.name, msg.name_size);
}

void send_empty(int sock, enum SOCKET_MSG_TYPE reply)
{
  struct SOCKET_MSG msg = { reply, 0, 0, 0, NULL, NULL };
  send_msg(sock, msg);
};

struct SOCKET_MSG get_msg(int sock)
{
  struct SOCKET_MSG msg;
  if (read(sock, &msg.type, sizeof(msg.type)) != sizeof(msg.type))
    MESSAGE_EXIT("Unknown message");
  if (read(sock, &msg.size, sizeof(msg.size)) != sizeof(msg.size))
    MESSAGE_EXIT("Unknown message");
  if (read(sock, &msg.name_size, sizeof(msg.name_size)) != sizeof(msg.name_size))
    MESSAGE_EXIT("Unknown message");
  if (read(sock, &msg.id, sizeof(msg.id)) != sizeof(msg.id))
    MESSAGE_EXIT("Unknown message");
  if (msg.size > 0) {
    msg.content = malloc(msg.size + 1);
    if (msg.content == NULL) ERROR_EXIT("");
    if (read(sock, msg.content, msg.size) != msg.size) {
      MESSAGE_EXIT("Unknown message");
    }
  } else {
    msg.content = NULL;
  }
  if (msg.name_size > 0) {
    msg.name = malloc(msg.name_size + 1);
    if (msg.name == NULL) ERROR_EXIT("");
    if (read(sock, msg.name, msg.name_size) != msg.name_size) {
      MESSAGE_EXIT("Unknown message");
    }
  } else {
    msg.name = NULL;
  }
  return msg;
}

void delete_msg(struct SOCKET_MSG msg)
{
  if (msg.content != NULL)
    free(msg.content);
  if (msg.name != NULL)
    free(msg.name);
}

void signal_handler(int signo)
{
  exit(0);
}

void cleanExit(void)
{
  close(web_socket);
  close(unix_socket);
  unlink(unix_socket_path);
  close(epoll);
}