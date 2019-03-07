#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zad1lib/library.h"

#define NUMBER_OF_COMMANDS 5

enum CMD {
    NO_COMMAND = -1,
    CREATE_TABLE = 0,
    SEARCH_DIRECTORY = 1,
    REMOVE_BLOCK = 2,
    EXIT = 3,
    ADD_TO_TABLE = 4,
};

struct Command {
    char *name;
    int numberOfArguments;
    enum CMD type;
    int (*fun)(struct Array *array, char **args);
};

size_t strToSizeT(char* str, int *error) {
    *error = 0;
    size_t ret = 0;
    int i = 0;
    //if (str[i] == '-')i++;
    while (str[i] != '\0') {
        if (str[i] < '0' || str[i] > '9')break;
        ret = ret * 10 + str[i] - '0';
        i++;
    }
    if (str[i] != '\0')*error = -1;
    //if(str[0]=='-')ret*=-1;
    return ret;
}

int createTable(struct Array *array, char **args) {
    size_t size;
    int error = 0;
    printf("%s",args[0]);
    size = strToSizeT(args[0], &error);
    printf("%lu", size);
    if (error < 0) {
        printf("Bad input");
        return 3;
    }
    if (size > 0) {
        if (array->size > 0)emptyArrayAndBlocks(array);
        if (createEmptyArray(array, size) != 0) {
            printf("Not enough memory for a table\n");
            return 4;
        }
        printf("Table created\n");
    }
    return -1;
}

int searchDirectory(struct Array *array, char **args) {
    char *dir;
    char *file;
    char *fileTmp;

    dir = args[0];
    file = args[1];
    fileTmp = args[2];

    if (dir == NULL || strlen(dir) == 0 || file == NULL || strlen(file) == 0 || fileTmp == NULL ||
        strlen(fileTmp) == 0)
        return 2;

    findAndSaveResultToTemporaryFile(dir, file, fileTmp);
}

int addToTable(struct Array *array, char **args) {
    const size_t nameFileTmpLength = 256;
    char fileTmp[nameFileTmpLength];
    scanf("%255s", fileTmp);
    if (fileTmp == NULL || strlen(fileTmp) == 0)
        return 1;
    addTemporaryFileBlockPointerToArray(array, fileTmp);
}

int my_exit(struct Array *array, char **args) {
    return 0;
}

const struct Command commands[NUMBER_OF_COMMANDS] = {
        {"create_table",     1, CREATE_TABLE, createTable},
        {"search_directory", 1, SEARCH_DIRECTORY, searchDirectory},
        {"remove_block",     1, REMOVE_BLOCK, NULL},
        {"exit",             0, EXIT, my_exit},
        {"add_to_table",     1, ADD_TO_TABLE, addToTable}
};

int processCommand(char *cmd, struct Array *array) {
    enum CMD currentCommand = NO_COMMAND;
    for (int i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if (strcmp(cmd, commands[i].name) == 0){
            char ** args = calloc(commands[i].numberOfArguments, sizeof(char*));
            for(int i=0;i<commands[i].numberOfArguments;i++){
                args[i]=calloc(4097, sizeof(char));
                scanf("%s", args[i]);
                printf("%s",args[i]);
            }
            return commands[i].fun(array, args);
        }
    }
    if(currentCommand == NO_COMMAND){
        printf("command %s not known", cmd);
        return 3;
    }
    return -1;
}

int main(int argc, char **argv) {
    struct Array array;
    int exitCode = -1;

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    char cmd[255];
    while (exitCode == -1) {
        printf("\n> ");
        scanf("%254s", cmd);
        exitCode = processCommand(cmd, &array);
    }
    return exitCode;
}