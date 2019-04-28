#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "communication.h"
#include "argsProcessor.h"

int serverQueueId = -1;
int running = 1;
int clientQueueId = -1;
int clientId = -1;
char* queueName;

int send(enum COMMAND type, char text[MESSAGE_SIZE]);

int runCommand(FILE *file);

int receive(struct MESSAGE *msg);

void do_read(char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], fileName[MAX_COMMAND_LENGTH];
    int numberOfArguments = sscanf(args, "%s %s", command, fileName);
    if (numberOfArguments == EOF || numberOfArguments < 2) {
        printf("Read expects file name");
        return;
    }
    FILE *f = fopen(fileName, "r");
    if (f == NULL) ERROR_EXIT("opening commands file");
    while (runCommand(f) != EOF);
    fclose(f);
}

void do_echo(char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MESSAGE_SIZE];
    int numberOfArguments = sscanf(args, "%s %s", command, text);
    if (numberOfArguments == EOF || numberOfArguments < 2) {
        printf("Echo expects text to write");
        return;
    }
    send(ECHO, text);
    struct MESSAGE msg;
    receive(&msg);
    if (msg.mType != ECHO) MESSAGE_EXIT("Wrong response type");
    printf("%s\n", msg.message);
}

void do_list() {
    send(LIST, "");
    struct MESSAGE msg;
    receive(&msg);
    if (msg.mType != LIST) MESSAGE_EXIT("Wrong response type");
    printf("%s\n", msg.message);
}

void do_stop() {
    printf("Stopping...\n");
    running = 0;
    send(STOP, "");
    exit(0);
}

void do_init() {
    struct MESSAGE msg;
    msg.mType = INIT;
    strcpy(msg.message, queueName);
    msg.senderId = getpid();
    if (mq_send(serverQueueId, (char *) &msg, MESSAGE_SIZE, commandPiority(INIT))) ERROR_EXIT("Sending");
    receive(&msg);
    if (msg.mType != INIT) MESSAGE_EXIT("Wrong response type");
    sscanf(msg.message, "%i", &clientId);
}

void do_friends(char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MESSAGE_SIZE];
    int numberOfArguments = sscanf(args, "%s %s", command, text);
    if (numberOfArguments == EOF || numberOfArguments == 0) MESSAGE_EXIT("Error in sscanf\n");
    send(FRIENDS, text);
}

void do_add(char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], list[MESSAGE_SIZE];
    int numberOfArguments = sscanf(args, "%s %s", command, list);
    if (numberOfArguments == EOF || numberOfArguments == 0) MESSAGE_EXIT("Error in sscanf\n");
    if (numberOfArguments == 1) {
        printf("ADD expects one argument");
        return;
    }
    send(ADD, list);
}

void do_del(char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], list[MESSAGE_SIZE];
    int numberOfArguments = sscanf(args, "%s %s", command, list);
    if (numberOfArguments == EOF || numberOfArguments == 0) MESSAGE_EXIT("Error in sscanf\n");
    if (numberOfArguments == 1) {
        printf("DEL expects one argument");
        return;
    }
    send(DEL, list);
}

void do_2_operation(enum COMMAND type, char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MESSAGE_SIZE];
    int numberOfArguments = sscanf(args, "%s %s", command, text);
    if (numberOfArguments == EOF || numberOfArguments < 2) MESSAGE_EXIT("2 command expects text to write");
    send(type, text);
}

void do_2_one(char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MESSAGE_SIZE];
    int receiverId;
    int numberOfArguments = sscanf(args, "%s %i %s", command, &receiverId, text);
    if (numberOfArguments == EOF || numberOfArguments < 3) {
        printf("2one command expects id and text to write\n");
        return;
    }
    sprintf(command, "%i %s", receiverId, text);
    send(_2ONE, command);
}

int runCommand(FILE *file) {
    char args[MAX_COMMAND_LENGTH], command[MAX_COMMAND_LENGTH];
    if (fgets(args, MAX_COMMAND_LENGTH * sizeof(char), file) == NULL)return EOF;
    int numberOfArguments = sscanf(args, "%s", command);
    if (numberOfArguments == EOF || numberOfArguments == 0)return 0;
    if (compare(command, "ECHO")) {
        do_echo(args);
    } else if (compare(command, "LIST")) {
        do_list();
    } else if (compare(command, "FRIENDS")) {
        do_friends(args);
    } else if (compare(command, "2ALL")) {
        do_2_operation(_2ALL, args);
    } else if (compare(command, "2FRIENDS")) {
        do_2_operation(_2FRIENDS, args);
    } else if (compare(command, "2ONE")) {
        do_2_one(args);
    } else if (compare(command, "STOP")) {
        do_stop();
        return EOF;
    } else if (compare(command, "READ")) {
        do_read(args);
    } else if (compare(command, "ADD")) {
        do_add(args);
    } else if (compare(command, "DEL")) {
        do_del(args);
    } else {
        printf("Unknown command\n");
    }
    return 0;
}

void cleanExit() {
    if (mq_close(serverQueueId) == -1) ERROR_EXIT("Closing server queue");
    if (mq_close(clientQueueId) == -1) ERROR_EXIT("Closing client queue");
    if (mq_unlink(queueName) == -1) ERROR_EXIT("Deleting queue");
    free(queueName);
}

void exitSignal(int signalno) {
    do_stop();
    exit(0);
}

void notifySignal(int signalno) {
    struct MESSAGE msg;
    receive(&msg);
    switch (msg.mType) {
        case _2ALL:
        case _2FRIENDS:
        case _2ONE:
            printf("%s", msg.message);
            break;
        case STOP:
            do_stop();
            break;
    }
}

int main(int argc, char **argv) {
    if (atexit(cleanExit) == -1) MESSAGE_EXIT("Registering atexit failed");
    signal(SIGRTMIN, notifySignal);
    signal(SIGINT, exitSignal);
    queueName=getClientQueueName();
    if ((serverQueueId = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1) ERROR_EXIT(
            "Creating queue");
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg = MAX_QUEUE_SIZE;
    queue_attr.mq_msgsize = MESSAGE_SIZE;
    if ((clientQueueId = mq_open(queueName, O_RDONLY | O_CREAT | O_EXCL, 0666, &queue_attr)) == -1) ERROR_EXIT(
            "Opening client queue");
    do_init();
    while (running) {
        runCommand(fdopen(STDIN_FILENO, "r"));
    }
    return 0;
}

int send(enum COMMAND type, char text[MESSAGE_SIZE]) {
    struct MESSAGE msg;
    msg.mType = type;
    strcpy(msg.message, text);
    msg.senderId = clientId;
    if (mq_send(serverQueueId, (char *) &msg, MESSAGE_SIZE, commandPiority(type)) == -1) ERROR_EXIT("Sending");
    return 0;
}

int receive(struct MESSAGE *msg) {
    if (mq_receive(clientQueueId, (char *) msg, MESSAGE_SIZE, NULL) == -1) ERROR_EXIT("Receiving response");
    return 0;
}