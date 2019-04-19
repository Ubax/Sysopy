#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "communication.h"
#include "argsProcessor.h"

enum INPUT_TYPE {
    FROM_FILE,
    FROM_TERMINAL
};

enum INPUT_TYPE input_type = FROM_TERMINAL;
char fileName[FILENAME_MAX];
int serverQueueId = -1;
int running = 1;
int clientQueueId = -1;
int clientId = -1;

int send(enum COMMAND type, char text[MESSAGE_SIZE]);

int runCommand(FILE *file);

int receive(struct MESSAGE *msg);

void do_read(char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], fileName[MAX_COMMAND_LENGTH];
    int numberOfArguments = sscanf(args, "%s %s", command, fileName);
    if (numberOfArguments == EOF || numberOfArguments < 2) MESSAGE_EXIT("Read expects file name");
    FILE *f = fopen(fileName, "r");
    while (runCommand(f) != EOF);
    fclose(f);
}

void do_echo(char args[MAX_COMMAND_LENGTH]) {
    char command[MAX_COMMAND_LENGTH], text[MESSAGE_SIZE];
    int numberOfArguments = sscanf(args, "%s %s", command, text);
    if (numberOfArguments == EOF || numberOfArguments < 2) MESSAGE_EXIT("Echo expects text to write");
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
    running = 0;
    send(STOP, "");
}

void do_init() {
    struct MESSAGE msg;
    char text[MESSAGE_SIZE];
    msg.mType = INIT;
    sprintf(text, "%i", clientQueueId);
    strcpy(msg.message, text);
    msg.senderId = clientId;
    if (msgsnd(serverQueueId, &msg, MSGSZ, IPC_NOWAIT)) ERROR_EXIT("Sending");
    receive(&msg);
    if (msg.mType != INIT) MESSAGE_EXIT("Wrong response type");
    sscanf(msg.message, "%i", &clientId);
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
        printf("friends\n");
    } else if (compare(command, "2ALL")) {
        printf("2all\n");
    } else if (compare(command, "2FRIENDS")) {
        printf("2friends\n");
    } else if (compare(command, "2ONE")) {
        printf("2one\n");
    } else if (compare(command, "STOP")) {
        do_stop();
        return EOF;
    } else if (compare(command, "READ")) {
        do_read(args);
    } else {
        printf("Unknown command\n");
    }
    return 0;
}

int main(int argc, char **argv) {
    if ((serverQueueId = msgget(getServerQueueKey(), 0)) == -1) ERROR_EXIT("Opening queue");
    if ((clientQueueId = msgget(getClientQueueKey(), IPC_CREAT | IPC_EXCL | 0666)) == -1) ERROR_EXIT(
            "Opening client queue");
    do_init();
    while (running) {
        runCommand(fdopen(STDIN_FILENO, "r"));
    }
    if (msgctl(clientQueueId, IPC_RMID, NULL) == -1) ERROR_EXIT("Deleting queue");
    return 0;
}

int send(enum COMMAND type, char text[MESSAGE_SIZE]) {
    struct MESSAGE msg;
    msg.mType = type;
    strcpy(msg.message, text);
    msg.senderId = clientId;
    if (msgsnd(serverQueueId, &msg, MSGSZ, IPC_NOWAIT)==-1) ERROR_EXIT("Sending");
    return 0;
}

int receive(struct MESSAGE *msg) {
    if (msgrcv(clientQueueId, msg, MSGSZ, 0, 0) == -1) ERROR_EXIT("Receiving response");
    return 0;
}