#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zad1lib/library.h"

#define NUMBER_OF_COMMANDS 5

const char *commands[NUMBER_OF_COMMANDS] = {
        "create_table",
        "search_directory",
        "remove_block",
        "exit",
        "add_to_table"
};

enum CMD {
    NO_COMMAND = -1,
    CREATE_TABLE = 0,
    SEARCH_DIRECTORY = 1,
    REMOVE_BLOCK = 2,
    EXIT = 3,
    ADD_TO_TABLE = 4,
};

size_t readSizeT(int *error) {
    *error = 0;
    size_t ret = 0;
    char str[13];
    scanf("%13s", str);
    int i = 0;
    //if (str[i] == '-')i++;
    while (i < 12 && str[i] != '\0') {
        if (str[i] < '0' || str[i] > '9')break;
        ret = ret * 10 + str[i] - '0';
        i++;
    }
    if (str[i] != '\0')*error = -1;
    //if(str[0]=='-')ret*=-1;
    return ret;
}

void createTable(struct Array* array) {
    size_t size;
    int error = 0;
    size = readSizeT(&error);
    if (error < 0) {
        printf("Bad input");
        return;
    }
    if (size > 0) {
        if (array != NULL)emptyArrayAndBlocks(array);
        if (createEmptyArray(array, size) != 0) {
            printf("Not enough memory for a table\n");
            return;
        }
        printf("Table created\n");
    }
}

void searchDirectory(struct Array* array) {
    const size_t dirLength = 4097;
    const size_t fileLength = 256;
    const size_t nameFileTmpLength = 256;

    char dir[dirLength];
    char file[fileLength];
    char fileTmp[nameFileTmpLength];

    scanf("%4096s", dir);
    scanf("%255s", file);
    scanf("%255s", fileTmp);

    if (dir == NULL || strlen(dir) == 0 || file == NULL || strlen(file) == 0 || fileTmp == NULL ||
        strlen(fileTmp) == 0)
        return;

    findAndSaveResultToTemporaryFile(dir, file, fileTmp);
}

void addToTable(struct Array* array) {
    const size_t nameFileTmpLength = 256;
    char fileTmp[nameFileTmpLength];
    scanf("%255s", fileTmp);
    if (fileTmp == NULL || strlen(fileTmp) == 0)
        return;
    addTemporaryFileBlockPointerToArray(array, fileTmp);
}

int processCommand(char * cmd, struct Array* array){
    enum CMD currentCommand = NO_COMMAND;
    for (int i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if (strcmp(cmd, commands[i]) == 0)currentCommand = i;
    }
    switch (currentCommand) {
        case CREATE_TABLE:
            createTable(array);
            break;
        case SEARCH_DIRECTORY:
            searchDirectory(array);
            break;
        case REMOVE_BLOCK:

            break;
        case ADD_TO_TABLE:
            addToTable(array);
            break;
        case EXIT:
            emptyArrayAndBlocks(array);
            return 0;
            break;
        default:
            printf("command %s not known", cmd);
    }
    return -1;
}

int main(int argc, char ** argv) {
    struct Array array;
    int exitCode = -1;

    for(int i=0;i<argc;i++){
        printf("%s\n", argv[i]);
    }

    char cmd[255];
    while (exitCode == -1) {
        printf("\n> ");
        scanf("%254s", cmd);
        exitCode = processCommand(cmd, &array);
    }
    return 0;
}