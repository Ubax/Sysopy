#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LENGTH 8096
#define NO_COMMAND 2
#define MAX_NUMBER_OF_COMMANDS 128
#define MAX_NUMBER_OF_ARGS 64

int analyseOperation(char *operation) {
    char *cmd = strtok(operation, " \t\n");
    if (cmd == NULL)return NO_COMMAND;
    printf("cmd: %s\t", cmd);
    char *arg = strtok(NULL, " \t\n");
    printf("args:");
    while (arg != NULL) {
        printf("%s#", arg);
        arg = strtok(NULL, " \t\n");
    }
    printf("\n");
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
    while (getline(&line, &size, file) != -1) {
        numberOfLine++;
        numberOfCommand = 0;
        operations[numberOfCommand] = strtok(line, "|");
        while (numberOfCommand < MAX_NUMBER_OF_COMMANDS && operations[numberOfCommand] != NULL) {
            operations[++numberOfCommand] = strtok(NULL, "|");
        }
        int i=0;
        if (numberOfCommand == 0)printf("No command specified in line: %i\n", numberOfLine);
        for (;i < numberOfCommand; i++) {
            analyseOperation(operations[i]);
        }
    }
    free(line);
    fclose(file);
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