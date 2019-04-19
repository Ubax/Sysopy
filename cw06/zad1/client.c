#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

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
    if(f==NULL)ERROR_EXIT("opening commands file");
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
    msg.senderId = getpid();
    if (msgsnd(serverQueueId, &msg, MSGSZ, IPC_NOWAIT)) ERROR_EXIT("Sending");
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

void do_2_operation(enum COMMAND type, char args[MAX_COMMAND_LENGTH]){
    char command[MAX_COMMAND_LENGTH], text[MESSAGE_SIZE];
    int numberOfArguments = sscanf(args, "%s %s", command, text);
    if (numberOfArguments == EOF || numberOfArguments < 2) MESSAGE_EXIT("2 command expects text to write");
    send(type, text);
}

void do_2_one(char args[MAX_COMMAND_LENGTH]){
    char command[MAX_COMMAND_LENGTH], text[MESSAGE_SIZE];
    int receiverId;
    int numberOfArguments = sscanf(args, "%s %i %s", command, &receiverId, text);
    if (numberOfArguments == EOF || numberOfArguments < 3) MESSAGE_EXIT("2one command expects id and text to write");
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
    } else {
        printf("Unknown command\n");
    }
    return 0;
}

void cleanExit(){
    if (msgctl(clientQueueId, IPC_RMID, NULL) == -1) ERROR_EXIT("Deleting queue");
    exit(0);
}

void exitSignal(int signalno){
    do_stop();
    cleanExit();
}

void notifySignal(int signalno){
    struct MESSAGE msg;
    receive(&msg);
    switch(msg.mType){
        case _2ALL:
        case _2FRIENDS:
        case _2ONE:
            printf("%s", msg.message);
            break;
        case STOP:
            cleanExit();
            break;
    }
}

int main(int argc, char **argv) {
    signal(SIGRTMIN, notifySignal);
    signal(SIGINT, exitSignal);
    if ((serverQueueId = msgget(getServerQueueKey(), 0)) == -1) ERROR_EXIT("Opening queue");
    if ((clientQueueId = msgget(getClientQueueKey(), IPC_CREAT | IPC_EXCL | 0666)) == -1) ERROR_EXIT(
            "Opening client queue");
    do_init();
    while (running) {
        runCommand(fdopen(STDIN_FILENO, "r"));
    }
    cleanExit();
    return 0;
}

int send(enum COMMAND type, char text[MESSAGE_SIZE]) {
    struct MESSAGE msg;
    msg.mType = type;
    strcpy(msg.message, text);
    msg.senderId = clientId;
    if (msgsnd(serverQueueId, &msg, MSGSZ, IPC_NOWAIT) == -1) ERROR_EXIT("Sending");
    return 0;
}

int receive(struct MESSAGE *msg) {
    if (msgrcv(clientQueueId, msg, MSGSZ, -(MAX_COMMAND_ID+1), 0) == -1) ERROR_EXIT("Receiving response");
    return 0;
}