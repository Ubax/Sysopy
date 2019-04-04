#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINE_MAX_LENGTH 8096
#define NO_COMMAND 2
#define MAX_NUMBER_OF_COMMANDS 128
#define MAX_NUMBER_OF_ARGS 64

struct Command {
    char *args[MAX_NUMBER_OF_ARGS];
    int size;
};

struct Command getCommand(char *operation) {
    struct Command command;
    command.size = 0;
    int i = 0;
    int wasLastWhitespace = 1;
    int isInText = 0;
    char bufor[strlen(operation) + 1];
    int buforIndex = 0;
    int wasLastEscape = 0;
    int j = 0;
    for (; j < MAX_NUMBER_OF_ARGS; j++) {
        command.args[j] = NULL;
    }

    for (; i < strlen(operation); i++) {
        if (!isInText) {
            if (operation[i] == ' ' || operation[i] == '\t' || operation[i] == '\n') {
                if (!wasLastWhitespace) {
                    bufor[buforIndex] = '\0';
                    if ((command.args[command.size] = malloc((buforIndex) * sizeof(char))) == NULL) {
                        perror("Command analysis");
                        exit(1);
                    }
                    strcpy(command.args[command.size], bufor);
                    command.size++;
                }
                buforIndex = 0;
                wasLastWhitespace = 1;
            } else {
                wasLastWhitespace = 0;
                bufor[buforIndex] = operation[i];
                buforIndex++;
            }
            if (operation[i] == '"')isInText = !isInText;
        } else {
            if (!wasLastEscape && operation[i] == '"')isInText = !isInText;
            wasLastWhitespace = 0;
            if (!wasLastEscape && operation[i] == '\\')wasLastEscape = 1;
            else {
                bufor[buforIndex] = operation[i];
                buforIndex++;
                wasLastEscape = 0;
            }
        }
    }
    return command;
}

int runCommands(struct Command commands[MAX_NUMBER_OF_COMMANDS], int numberOfCommands) {
    int oldFd[2];
    int newFd[2];
    int i = 0;
    for (; i < numberOfCommands; i++) {
        oldFd[0] = newFd[0];
        if (pipe(newFd) == -1) {
            perror("Run command");
            exit(1);
        }
        pid_t pid = fork();
        if (pid == 0) {
            close(newFd[0]);
            if (i < numberOfCommands - 1)dup2(newFd[1], STDOUT_FILENO);
            if (i > 0)dup2(oldFd[0], STDIN_FILENO);

            execvp(commands[i].args[0], commands[i].args);
            exit(EXIT_SUCCESS);
        }
        close(oldFd[0]);
        close(newFd[1]);
    }
    for (; i < numberOfCommands; i++)wait(NULL);
    return 0;
}

int analyseFile(char *fileName) {
    char *line = NULL;
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Analysing file");
        return 1;
    }
    size_t size = LINE_MAX_LENGTH - 1;
    int numberOfLine = 0;
    int numberOfCommand = 0;
    char *operations[MAX_NUMBER_OF_COMMANDS + 1];
    struct Command commands[MAX_NUMBER_OF_COMMANDS];

    while (getline(&line, &size, file) != -1) {
        numberOfLine++;
        numberOfCommand = 0;

        operations[numberOfCommand] = strtok(line, "|");
        while (numberOfCommand < MAX_NUMBER_OF_COMMANDS && operations[numberOfCommand] != NULL) {
            operations[++numberOfCommand] = strtok(NULL, "|");
        }
        if (numberOfCommand == 0)printf("No command specified in line: %i\n", numberOfLine);

        int i = 0;
        for (; i < numberOfCommand; i++) {
            struct Command command = getCommand(operations[i]);
            commands[i] = command;
        }
        runCommands(commands, numberOfCommand);
    }
    free(line);
    fclose(file);
    if (numberOfLine == 0) {
        printf("No lines\n");
        perror("Analysing file");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Program expects at last 1 argument: [file name]\n");
        return 1;
    }
    if (analyseFile(argv[1]))return 1;
    return 0;
}