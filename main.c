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

size_t strToSizeT(char *str, int *error) {
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
    printf("\tcreate_table...\n");
    size_t size;
    int error = 0;
    printf("\t\targs: %s\n", args[0]);
    size = strToSizeT(args[0], &error);
    printf("\t\tafter conversion: %lu\n", size);
    if (error < 0) {
        printf("Bad input\n");
        return 4;
    }
    if (size > 0) {
        printf("\t\tsize>0: true\n");
        if (array->size > 0)emptyArrayAndBlocks(array);
        if (createEmptyArray(array, size) != 0) {
            printf("Not enough memory for a table\n");
            return 4;
        }
        printf("\tTable created\n");
    }
    return -1;
}

int searchDirectory(struct Array *array, char **args) {
    printf("\tsearch_directory...\n");
    char *dir;
    char *file;
    char *fileTmp;

    dir = args[0];
    file = args[1];
    fileTmp = args[2];

    if (dir == NULL || strlen(dir) == 0 || file == NULL || strlen(file) == 0 || fileTmp == NULL ||
        strlen(fileTmp) == 0)
        return 3;
    printf("\tall arguments seem to be correct...\n");

    findAndSaveResultToTemporaryFile(dir, file, fileTmp);
    return -1;
}

int addToTable(struct Array *array, char **args) {
    printf("\tadd_to_table...\n");
    const size_t nameFileTmpLength = 256;
    char *fileTmp = args[0];
    if (fileTmp == NULL || strlen(fileTmp) == 0)
        return 2;
    printf("\tall arguments seem to be correct...\n");
    addTemporaryFileBlockPointerToArray(array, fileTmp);
    return -1;
}

int my_exit(struct Array *array, char **args) {
    return 0;
}

int removeBlock(struct Array *array, char **args) {
    size_t index;
    int error = 0;
    printf("\t\targs: %s\n", args[0]);
    index = strToSizeT(args[0], &error);
    printf("\t\tafter conversion: %lu\n", index);
    if (error < 0) {
        printf("Bad input\n");
        return 4;
    }
    if (index >= 0) {
        printf("\t\tindex>=0: true\n");
        deleteBlockFromArray(array, index);
    }
    return -1;
}

const struct Command commands[NUMBER_OF_COMMANDS] = {
        {"create_table",     1, CREATE_TABLE,     createTable},
        {"search_directory", 3, SEARCH_DIRECTORY, searchDirectory},
        {"remove_block",     1, REMOVE_BLOCK,     removeBlock},
        {"exit",             0, EXIT,             my_exit},
        {"add_to_table",     1, ADD_TO_TABLE,     addToTable}
};

int console(struct Array *array) {
    char cmd[255];
    scanf("%254s", cmd);
    enum CMD currentCommand = NO_COMMAND;
    for (int numberOfCommand = 0; numberOfCommand < NUMBER_OF_COMMANDS; numberOfCommand++) {
        if (strcmp(cmd, commands[numberOfCommand].name) == 0) {
            currentCommand = commands->type;
            char **args = NULL;
            args = calloc(commands[numberOfCommand].numberOfArguments, sizeof(char *));
            for (int numberOfCommandArgument = 0;
                 numberOfCommandArgument < commands[numberOfCommand].numberOfArguments; numberOfCommandArgument++) {
                args[numberOfCommandArgument] = calloc(4097, sizeof(char));
                scanf("%s", args[numberOfCommandArgument]);
            }
            int ret = commands[numberOfCommand].fun(array, args);
            if (args != NULL)free(args);
            return ret;
        }
    }
    if (currentCommand == NO_COMMAND) {
        printf("command %s not known", cmd);
        return 1;
    }
    return -1;
}

int argumentList(int argc, char **argv, struct Array *array) {
    printf("started processing...\n");
    int numberOfListArgument = 1;
    for (; numberOfListArgument < argc; numberOfListArgument++) {
        enum CMD currentCommand = NO_COMMAND;
        for (int numberOfCommand = 0; numberOfCommand < NUMBER_OF_COMMANDS; numberOfCommand++) {
            if (strcmp(argv[numberOfListArgument], commands[numberOfCommand].name) == 0) {
                printf("%s...\n", commands[numberOfCommand].name);
                currentCommand = commands->type;
                char **args = NULL;
                args = calloc(commands[numberOfCommand].numberOfArguments, sizeof(char *));
                for (int numberOfCommandArgument = 0;
                     numberOfCommandArgument < commands[numberOfCommand].numberOfArguments; numberOfCommandArgument++) {
                    numberOfListArgument++;
                    args[numberOfCommandArgument] = argv[numberOfListArgument];
                }
                int ret = commands[numberOfCommand].fun(array, args);
                if (args != NULL)free(args);
                if (ret >= 0)return ret;
            }
        }
        if (currentCommand == NO_COMMAND) {
            printf("command %s not known", argv[numberOfListArgument]);
            return 1;
        }
    }
    return -1;
}

int main(int argc, char **argv) {
    printf("Systemy Operacyjne 2019\nAuthor: Jakub Tkacz\nVesrion:1.0\nDate:07.03.2019\n\n");
    struct Array array = {NULL, 0};
    int exitCode = argumentList(argc, argv, &array);

    while (exitCode == -1) {
        printf("\n> ");
        exitCode = console(&array);
    }
    return exitCode;
}