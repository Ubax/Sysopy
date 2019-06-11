#include "client.h"
#include "sysopy.h"

void cleanExit();

char *name;
char *address;
enum CONNECTION_TYPE connection_type = UNIX;
int socket_fd;

int kill_from_server = 0;

void init();

struct SOCKET_MSG get_msg(void);

void delete_msg(struct SOCKET_MSG msg);

void send_msg(struct SOCKET_MSG msg);

void send_empty(enum SOCKET_MSG_TYPE);

void send_done(int, char *);

void signal_handler(int);

void processMSG(struct SOCKET_MSG msg);

int main(int argc, char *argv[]) {
    if (atexit(cleanExit) == -1) MESSAGE_EXIT("Registering atexit failed");
    if (signal(SIGINT, signal_handler) == SIG_ERR) ERROR_EXIT("Signal registering");
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

    init();

    while (1) {
        struct SOCKET_MSG msg = get_msg();

        processMSG(msg);

        delete_msg(msg);
    }
}

void processMSG(struct SOCKET_MSG msg) {
    switch (msg.type) {
        case OK: {
            printf("Ok :)\n");
            break;
        }
        case PING: {
            send_empty(PONG);
            break;
        }
        case NAME_TAKEN: MESSAGE_EXIT("Name is already taken");
        case FULL: MESSAGE_EXIT("Server is full");
        case WORK: {
            printf("Some work came\n");
            char *buffer = malloc(100 + 2 * msg.size);
            if (buffer == NULL) ERROR_EXIT("Allocating buffer");
            sprintf(buffer, "echo '%s' | awk '{l=split($0,res,\" \");for(i=0;i <= l; i++)printf(\"%%s\\n\",res[i])}' | sort | uniq -c | sort -n", (char *) msg.content);
            FILE *result = popen(buffer, "r");
            if (result == 0) {
                free(buffer);
                break;
            }
            int n = fread(buffer, 1, 99 + 2 * msg.size, result);
            buffer[n] = '\0';
            printf("Done it\n");
            send_done(msg.id, buffer);
            free(buffer);
            break;
        }
        case STOP:
            kill_from_server = 1;
            exit(0);
        default:
            break;
    }
}

void init() {
    if (connection_type == NETWORK) {
        strtok(address, ":");
        char *port = strtok(NULL, ":");
        if (port == NULL) MESSAGE_EXIT("No port in address");

        uint32_t in_addr = inet_addr(address);
        if (in_addr == INADDR_NONE) MESSAGE_EXIT("Invalid ip address");

        uint16_t port_num = (uint16_t) atoi(port);
        if (port_num < 1024) MESSAGE_EXIT("Too low port number");

        if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) ERROR_EXIT("Creating web socket");

        struct sockaddr_in web_addr;
        memset(&web_addr, 0, sizeof(struct sockaddr_in));

        web_addr.sin_family = AF_INET;
        web_addr.sin_addr.s_addr = in_addr;
        web_addr.sin_port = htons(port_num);

        if (connect(socket_fd, (const struct sockaddr *) &web_addr, sizeof(web_addr)) == -1) ERROR_EXIT(
                "Connecting web socket");
    } else if (connection_type == UNIX) {
        char *un_path = address;

        if (strlen(un_path) < 1 || strlen(un_path) > UNIX_PATH_MAX) MESSAGE_EXIT("Too long unix socket path");

        struct sockaddr_un un_addr;
        un_addr.sun_family = AF_UNIX;
        snprintf(un_addr.sun_path, UNIX_PATH_MAX, "%s", un_path);

        struct sockaddr_un client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sun_family = AF_UNIX;
        snprintf(client_addr.sun_path, UNIX_PATH_MAX, "%s", name);

        if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) ERROR_EXIT("Creating unix socket");

        if (bind(socket_fd, (const struct sockaddr *) &client_addr, sizeof(client_addr)) == -1) ERROR_EXIT(
                "Binding unix socket");

        if (connect(socket_fd, (const struct sockaddr *) &un_addr, sizeof(un_addr)) == -1) {
            unlink(name);
            ERROR_EXIT("Connecting unix socket");
        }

    }

    send_empty(REGISTER);
}

void send_msg(struct SOCKET_MSG msg) {
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

    if (write(socket_fd, buff, size) != size) ERROR_EXIT("Writing to socket");

    free(buff);
}

void send_empty(enum SOCKET_MSG_TYPE type) {
    struct SOCKET_MSG msg = {type, 0, strlen(name) + 1, 0, NULL, name};
    send_msg(msg);
};

void send_done(int id, char *content) {
    struct SOCKET_MSG msg = {WORK_DONE, strlen(content) + 1, strlen(name) + 1, id, content, name};
    send_msg(msg);
}

struct SOCKET_MSG get_msg(void) {
    struct SOCKET_MSG msg;
    ssize_t head_size = sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size) + sizeof(msg.id);
    int8_t buff[head_size];
    if (recv(socket_fd, buff, head_size, MSG_PEEK) < head_size) MESSAGE_EXIT("Uknown message from server");

    memcpy(&msg.type, buff, sizeof(msg.type));
    memcpy(&msg.size, buff + sizeof(msg.type), sizeof(msg.size));
    memcpy(&msg.name_size, buff + sizeof(msg.type) + sizeof(msg.size), sizeof(msg.name_size));
    memcpy(&msg.id, buff + sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size), sizeof(msg.id));

    ssize_t size = head_size + msg.size + 1 + msg.name_size + 1;
    int8_t *buffer = malloc(size);

    if (recv(socket_fd, buffer, size, 0) < size) {
        MESSAGE_EXIT("Unknown message from server");
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
    if(!kill_from_server)send_empty(UNREGISTER);
    unlink(name);
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}
