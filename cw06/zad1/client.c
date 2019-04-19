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
int queueId = -1;
int running = 1;


int send(struct MESSAGE *msg);
int runCommand(FILE* file);

void do_read(char args[MAX_COMMAND_LENGTH]){
    char command[MAX_COMMAND_LENGTH], fileName[MAX_COMMAND_LENGTH];
    int numberOfArguments = sscanf(args, "%s %s", command, fileName);
    if(numberOfArguments==EOF || numberOfArguments < 2)MESSAGE_EXIT("Read expects file name");
    FILE *f = fopen(fileName, "r");
    while (runCommand(f)!=EOF);
    fclose(f);
}

void do_echo(char args[MAX_COMMAND_LENGTH]){
    char command[MAX_COMMAND_LENGTH], text[MAX_COMMAND_LENGTH];
    int numberOfArguments = sscanf(args, "%s %s", command, text);
    if(numberOfArguments==EOF || numberOfArguments < 2)MESSAGE_EXIT("Echo expects text to write");
    printf("%s\n", text);
}

int runCommand(FILE* file) {
    char args[MAX_COMMAND_LENGTH], command[MAX_COMMAND_LENGTH];
    if(fgets(args, MAX_COMMAND_LENGTH* sizeof(char), file)==NULL)return EOF;
    int numberOfArguments = sscanf(args, "%s", command);
    if(numberOfArguments==EOF || numberOfArguments == 0)return 0;
    if (compare(command, "ECHO")) {
        do_echo(args);
    } else if (compare(command, "LIST")) {
        printf("list\n");
    } else if (compare(command, "FRIENDS")) {
        printf("friends\n");
    } else if (compare(command, "2ALL")) {
        printf("2all\n");
    } else if (compare(command, "2FRIENDS")) {
        printf("2friends\n");
    } else if (compare(command, "2ONE")) {
        printf("2one\n");
    } else if (compare(command, "STOP")) {
        running=0;
        return EOF;
    } else if(compare(command, "READ")){
        do_read(args);
    } else {
        printf("Unknown command\n");
    }
    return 0;
}

int main(int argc, char **argv) {
    //if((queueId = msgget(getServerQueueKey(), 0))==-1)ERROR_EXIT("Opening queue");
    while(running){
        runCommand(fdopen(STDIN_FILENO, "r"));
    }
    return 0;
}

int send(struct MESSAGE *msg) {
    if(msgsnd(queueId, msg, MSGSZ, IPC_NOWAIT))
        ERROR_EXIT("Sending");
    return 0;
}