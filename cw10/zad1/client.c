#include "client.h"
#include "sysopy.h"

void cleanExit();

char *name;
char *address;
enum CONNECTION_TYPE connection_type = UNIX;
int socket_fd;

void init();

struct SOCKET_MSG get_msg(void);

void delete_msg(struct SOCKET_MSG msg);

void send_msg(struct SOCKET_MSG *);

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

    if(connection_type == UNIX)printf("Running in unix mode\n");
    else printf("Running in network mode\n");

    while (1) {
        struct SOCKET_MSG msg = get_msg();

        processMSG(msg);

        delete_msg(msg);
    }
}

void processMSG(struct SOCKET_MSG msg) {
    switch (msg.type) {
        case OK: {
            printf("OK :)\n");
            break;
        }
        case PING: {
            send_empty(PONG);
            break;
        }
        case NAME_TAKEN:
            MESSAGE_EXIT("Name is already taken");
        case FULL:
            MESSAGE_EXIT("Server is full");
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
        default:
            break;
    }
}

void init() {
    if (connection_type == NETWORK) {
        strtok(address, ":");
        char *port = strtok(NULL, ":");
        if (port == NULL) MESSAGE_EXIT("Specify a port");

        uint32_t in_addr = inet_addr(address);
        if (in_addr == INADDR_NONE) MESSAGE_EXIT("Invalid address");

        uint16_t port_num = (uint16_t) atoi(port);
        if (port_num < 1024)
            MESSAGE_EXIT("Invalid port number");

        struct sockaddr_in web_addr;
        memset(&web_addr, 0, sizeof(struct sockaddr_in));

        web_addr.sin_family = AF_INET;
        web_addr.sin_addr.s_addr = in_addr;
        web_addr.sin_port = htons(port_num);

        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            ERROR_EXIT("Creating web socket");

        if (connect(socket_fd, (const struct sockaddr *) &web_addr, sizeof(web_addr)) == -1)
            ERROR_EXIT("Connecting web socket");
    } else if (connection_type == UNIX) {
        char *un_path = address;

        if (strlen(un_path) < 1 || strlen(un_path) > UNIX_PATH_MAX)
            MESSAGE_EXIT("Invalid unix socket path");

        struct sockaddr_un un_addr;
        un_addr.sun_family = AF_UNIX;
        snprintf(un_addr.sun_path, UNIX_PATH_MAX, "%s", un_path);

        if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
            ERROR_EXIT("Creating unix socket");

        if (connect(socket_fd, (const struct sockaddr *) &un_addr, sizeof(un_addr)) == -1)
            ERROR_EXIT("Connecting unix socket");
    }

    send_empty(REGISTER);
}

void send_msg(struct SOCKET_MSG *msg) {
    write(socket_fd, &msg->type, sizeof(msg->type));
    write(socket_fd, &msg->size, sizeof(msg->size));
    write(socket_fd, &msg->name_size, sizeof(msg->name_size));
    write(socket_fd, &msg->id, sizeof(msg->id));
    if (msg->size > 0) write(socket_fd, msg->content, msg->size);
    if (msg->name_size > 0) write(socket_fd, msg->name, msg->name_size);
}

void send_empty(enum SOCKET_MSG_TYPE type) {
    struct SOCKET_MSG msg = {type, 0, strlen(name) + 1, 0, NULL, name};
    send_msg(&msg);
};

void send_done(int id, char *content) {
    struct SOCKET_MSG msg = {WORK_DONE, strlen(content) + 1, strlen(name) + 1, id, content, name};
    send_msg(&msg);
}

struct SOCKET_MSG get_msg(void) {
    struct SOCKET_MSG msg;
    if (read(socket_fd, &msg.type, sizeof(msg.type)) != sizeof(msg.type))
        MESSAGE_EXIT("Unknown message from server");
    if (read(socket_fd, &msg.size, sizeof(msg.size)) != sizeof(msg.size))
        MESSAGE_EXIT("Unknown message from server");
    if (read(socket_fd, &msg.name_size, sizeof(msg.name_size)) != sizeof(msg.name_size))
        MESSAGE_EXIT("Unknown message from server");
    if (read(socket_fd, &msg.id, sizeof(msg.id)) != sizeof(msg.id))
        MESSAGE_EXIT("Unknown message from server");
    if (msg.size > 0) {
        msg.content = malloc(msg.size + 1);
        if (msg.content == NULL) ERROR_EXIT("Allocating message content");
        if (read(socket_fd, msg.content, msg.size) != msg.size) {
            MESSAGE_EXIT("Unknown message from server");
        }
    } else {
        msg.content = NULL;
    }
    if (msg.name_size > 0) {
        msg.name = malloc(msg.name_size + 1);
        if (msg.name == NULL) ERROR_EXIT("Allocating message name");
        if (read(socket_fd, msg.name, msg.name_size) != msg.name_size) {
            MESSAGE_EXIT("Unknown message from server");
        }
    } else {
        msg.name = NULL;
    }
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
    send_empty(UNREGISTER);
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}
