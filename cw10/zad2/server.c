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
pthread_t commands_thread;

void init();

void *commands_fun(void *arg);

void *ping_fun(void *arg);

void handle_call(int socket);

void send_msg(int i, struct SOCKET_MSG msg);

void send_empty(int i, enum SOCKET_MSG_TYPE);

void delete_client(int i);

int client_by_name(char *name);

struct SOCKET_MSG get_msg(int socket, struct sockaddr *addr, socklen_t *addr_len);

void delete_msg(struct SOCKET_MSG msg);

void signal_handler(int signo);

void cleanExit();

int main(int argc, char *argv[]) {
    if (atexit(cleanExit) == -1) MESSAGE_EXIT("Registering atexit failed");
    if(signal(SIGINT, signal_handler) == SIG_ERR)MESSAGE_EXIT("Registering signal failed");
    if (argc < 2) {
        MESSAGE_EXIT("Program expects 2 arguments: "
                     "TCP/UDP port number\t"
                     "UNIX socket path");
    }
    port_number = (__uint16_t) getArgAsInt(argv, 1);
    if (port_number < 1024 || port_number > 65535) MESSAGE_EXIT("Port number out of range [1024, 65535]");
    if (strlen(argv[2]) > UNIX_PATH_MAX) MESSAGE_EXIT("Too long path name");
    unix_socket_path = argv[2];

    init();

    struct epoll_event event;
    while (1) {
        if (epoll_wait(epoll, &event, 1, -1) == -1) ERROR_EXIT("Waiting for event");
        handle_call(event.data.fd);
    }
}

void init() {
    struct sockaddr_in web_addr;
    memset(&web_addr, 0, sizeof(struct sockaddr_in));
    web_addr.sin_family = AF_INET;
    web_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    web_addr.sin_port = htons(port_number);

    if ((web_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) ERROR_EXIT("Creating web socket");

    if (bind(web_socket, (const struct sockaddr *) &web_addr, sizeof(web_addr))) ERROR_EXIT("Binding web socket");

    struct sockaddr_un un_addr;
    un_addr.sun_family = AF_UNIX;

    snprintf(un_addr.sun_path, UNIX_PATH_MAX, "%s", unix_socket_path);

    if ((unix_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) ERROR_EXIT("Creating unix socket");

    if (bind(unix_socket, (const struct sockaddr *) &un_addr, sizeof(un_addr))) ERROR_EXIT("Binding unix socket");

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;

    if ((epoll = epoll_create1(0)) == -1) ERROR_EXIT("Creating epoll");

    event.data.fd = web_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, web_socket, &event) == -1) ERROR_EXIT("Adding web socket to epoll");

    event.data.fd = unix_socket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, unix_socket, &event) == -1) ERROR_EXIT("Adding unix socket to epoll");

    // start threads
    if (pthread_create(&commands_thread, NULL, commands_fun, NULL) != 0) ERROR_EXIT("Creating commands thread");
    if (pthread_detach(commands_thread) != 0) ERROR_EXIT("Detaching commands thread");

    if (pthread_create(&pinger, NULL, ping_fun, NULL) != 0) ERROR_EXIT("Creating ping thread");
    if (pthread_detach(pinger) != 0) ERROR_EXIT("Detaching ping thread");
}

int readFile(char *name, char **_buffer) {
    char *buffer;
    FILE *file = fopen(name, "r");
    if (file == NULL) {
        perror("Opening file");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    buffer = malloc(size + 1);
    if (buffer == NULL) {
        perror("Allocating buffer");
        return 1;
    }

    buffer[size] = '\0';

    if (fread(buffer, 1, size, file) != size) {
        fprintf(stderr, "Could not read file\n");
        free(buffer);
        return 1;
    }

    fclose(file);
    *_buffer = buffer;
    return 0;
}

void *commands_fun(void *args) {
    char file_name[1024];
    while (1) {
        int min_i = MAX_CLIENTS_NUMBER;
        int min = 1000000;

        int min_not_active = 1000000;
        int min_i_not_active = MAX_CLIENTS_NUMBER;

        // read command
        scanf("%1023s", file_name);

        // open file
        char *file_buffer = NULL;
        if(readFile(file_name, &file_buffer))continue;
        printf("%s\n", file_buffer);

        // send request
        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
            if (!clients[i].socketFD) continue;
            if (min > clients[i].working) {
                min_i = i;
                min = clients[i].working;
            }
            if(min_not_active > clients[i].working && clients[i].currently_working){
                min_not_active = clients[i].working;
                min_i_not_active = i;
            }
        }
//        if(min_i != min_i_not_active){
//            min_i = min_i_not_active;
//        }

        if (min_i < MAX_CLIENTS_NUMBER) {
            struct SOCKET_MSG msg = {WORK, strlen(file_buffer) + 1, 0, ++id, file_buffer, NULL};
            printf("Job id: %lu \tsend to client:  %s\n", id, clients[min_i].name);
            send_msg(min_i, msg);
            clients[min_i].working++;
            clients[min_i].currently_working++;
        } else {
            fprintf(stderr, "No clients connected\n");
        }
        pthread_mutex_unlock(&client_mutex);

        free(file_buffer);
    }
}

void *ping_fun(void *args) {
    while (1) {
        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
            if (clients[i].socketFD == 0) continue;
            if (clients[i].inactive) {
                delete_client(i);
            } else {
                clients[i].inactive = 1;
                send_empty(i, PING);
            }
        }
        pthread_mutex_unlock(&client_mutex);
        sleep(10);
    }
}

void process_msg(struct SOCKET_MSG msg, struct sockaddr *addr, socklen_t addr_len, int socket) {
    printf("Process msg\n");
    switch (msg.type) {
        case REGISTER: {
            printf("Register...\n");
            enum SOCKET_MSG_TYPE reply = OK;
            int i;
            i = client_by_name(msg.name);
            if (i < MAX_CLIENTS_NUMBER)
                reply = NAME_TAKEN;

            for (i = 0; i < MAX_CLIENTS_NUMBER && clients[i].socketFD != 0; i++);

            if (i == MAX_CLIENTS_NUMBER)
                reply = FULL;

            if (reply != OK) {
                send_empty(socket, reply);
                break;
            }

            clients[i].socketFD = socket;
            clients[i].name = malloc(msg.name_size + 1);
            if (clients[i].name == NULL) ERROR_EXIT("Allocating client name");
            strcpy(clients[i].name, msg.name);
            clients[i].addr = addr;
            clients[i].addr_len = addr_len;
            if(i>0)clients[i].working = clients[i-1].working;
            else clients[i].working = 0;
            clients[i].inactive = 0;
            clients[i].currently_working=0;

            send_empty(i, OK);
            break;
        }
        case UNREGISTER: {
            int i;
            for (i = 0;
                 i < MAX_CLIENTS_NUMBER && (clients[i].socketFD == 0 || strcmp(clients[i].name, msg.name) != 0); i++);
            if (i == MAX_CLIENTS_NUMBER) break;
            printf("Unregistering client %s\n", clients[i].name);
            delete_client(i);
            break;
        }
        case WORK_DONE: {
            int i = client_by_name(msg.name);
            if (i < MAX_CLIENTS_NUMBER) {
                clients[i].inactive = 0;
                clients[i].currently_working--;
            }
            printf("JOB %lu DONE BY %s:\n%s\n", msg.id, (char *) msg.name, (char *) msg.content);
            break;
        }
        case PONG: {
            int i = client_by_name(msg.name);
            if (i < MAX_CLIENTS_NUMBER)
                clients[i].inactive = 0;
        }
    }
}

void handle_call(int sock) {
    struct sockaddr *addr = malloc(sizeof(struct sockaddr));
    if (addr == NULL) ERROR_EXIT("Empty address");
    socklen_t addr_len = sizeof(struct sockaddr);
    struct SOCKET_MSG msg = get_msg(sock, addr, &addr_len);
    pthread_mutex_lock(&client_mutex);

    process_msg(msg, addr, addr_len, sock);

    pthread_mutex_unlock(&client_mutex);

    delete_msg(msg);
}

void delete_client(int i) {
    clients[i].socketFD = 0;
    free(clients[i].name);
    clients[i].name = NULL;
    free(clients[i].addr);
    clients[i].addr = NULL;
    clients[i].addr_len = 0;
    clients[i].working = 0;
    clients[i].inactive = 0;
}

int client_by_name(char *name) {
    int i;
    for (i = 0; i < MAX_CLIENTS_NUMBER; i++) {
        if (clients[i].socketFD == 0) continue;
        if (strcmp(clients[i].name, name) == 0)
            break;
    }
    return i;
}

void send_msg(int i, struct SOCKET_MSG msg) {
    ssize_t head_size = sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size) + sizeof(msg.id);
    ssize_t size = head_size + msg.size + 1 + msg.name_size + 1;
    int8_t *buff = malloc(size);
    if (buff == NULL) ERROR_EXIT("Allocating message buffer");

    memcpy(buff, &msg.type, sizeof(msg.type));
    memcpy(buff + sizeof(msg.type), &msg.size, sizeof(msg.size));
    memcpy(buff + sizeof(msg.type) + sizeof(msg.size), &msg.name_size, sizeof(msg.name_size));
    memcpy(buff + sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size), &msg.id, sizeof(msg.id));

    if (msg.size > 0 && msg.content != NULL)
        memcpy(buff + head_size, msg.content, msg.size + 1);
    if (msg.name_size > 0 && msg.name != NULL)
        memcpy(buff + head_size + msg.size + 1, msg.name, msg.name_size + 1);

    sendto(clients[i].socketFD, buff, size, 0, clients[i].addr, clients[i].addr_len);

    free(buff);
}

void send_empty(int i, enum SOCKET_MSG_TYPE reply) {
    struct SOCKET_MSG msg = {reply, 0, 0, 0, NULL, NULL};
    send_msg(i, msg);
};

struct SOCKET_MSG get_msg(int socket_fd, struct sockaddr *addr, socklen_t *addr_len) {
    struct SOCKET_MSG msg;
    ssize_t head_size = sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size) + sizeof(msg.id);
    int8_t buff[head_size];
    if (recv(socket_fd, buff, head_size, MSG_PEEK) < head_size) MESSAGE_EXIT("Uknown message from client");

    memcpy(&msg.type, buff, sizeof(msg.type));
    memcpy(&msg.size, buff + sizeof(msg.type), sizeof(msg.size));
    memcpy(&msg.name_size, buff + sizeof(msg.type) + sizeof(msg.size), sizeof(msg.name_size));
    memcpy(&msg.id, buff + sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size), sizeof(msg.id));

    ssize_t size = head_size + msg.size + 1 + msg.name_size + 1;
    int8_t *buffer = malloc(size);

    if (recvfrom(socket_fd, buffer, size, 0, addr, addr_len) < size) {
        MESSAGE_EXIT("Uknown message from client");
    }

    if (msg.size > 0) {
        msg.content = malloc(msg.size + 1);
        if (msg.content == NULL) ERROR_EXIT("Allocating message content");
        memcpy(msg.content, buffer + head_size, msg.size + 1);
    } else {
        msg.content = NULL;
    }

    if (msg.name_size > 0) {
        msg.name = malloc(msg.name_size + 1);
        if (msg.name == NULL) ERROR_EXIT("Allocating message name");
        memcpy(msg.name, buffer + head_size + msg.size + 1, msg.name_size + 1);
    } else {
        msg.name = NULL;
    }

    free(buffer);

    return msg;
}

void delete_msg(struct SOCKET_MSG msg) {
    if (msg.content != NULL)
        free(msg.content);
    if (msg.name != NULL)
        free(msg.name);
}

void signal_handler(int signo) {
    exit(0);
}

void cleanExit(void) {
    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
        if (clients[i].socketFD){
            send_empty(i, STOP);
        }
    }
    close(web_socket);
    close(unix_socket);
    unlink(unix_socket_path);
    close(epoll);
}