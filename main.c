#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_ARRAY_LENGTH 20
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

void findAndSaveResultToTemporaryFile(char *directory, char *fileName, char *tempFileName) {
    char command[strlen(directory) + strlen(fileName) + 30];
    strcpy(command, "find ");
    strcat(command, directory);
    strcat(command, " -name \"");
    strcat(command, fileName);
    strcat(command, "\" > ");
    strcat(command, tempFileName);
    system(command);
}

size_t fileSize(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

char *readTemporaryFile(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL){
        printf("No temporary file with that name\n");
        return NULL;
    }

    char *block = (char *) calloc(fileSize(file), sizeof(char));

    char c;
    int i = 0;
    while ((c = fgetc(file)) != EOF) {
        block[i] = c;
        i++;
    }
    return block;
}

int createEmptyArray(char **array, size_t *arraySize, size_t size) {
    array = calloc(size, sizeof(char *));
    if (array == NULL)return -1;
    for (int i = 0; i < size; i++)array[i] = NULL;
    (*arraySize) = size;
    return 0;
}

int createEmptyDefaultSizeArray(char **array, size_t *arraySize) {
    return createEmptyArray(array, arraySize, INITIAL_ARRAY_LENGTH);
}

int addTemporaryFileBlockPointerToArray(char **array, size_t *arraySize, char *tempFileName) {
    int i = 0;
    if(*arraySize<1 || array == NULL)return -1;
    for (; i < (*arraySize); i++) {
        if (array[i] == NULL)break;
    }
    if (i == (*arraySize))
        if (!realloc(array, ++(*arraySize)))return -1;
    array[i] = readTemporaryFile(tempFileName);
    return i;
}

void deleteBlockFromArray(char **array, size_t *arraySize, int i) {
    if (i >= *arraySize || array[i] == NULL)return;
    free(array[i]);
    array[i] = NULL;
}

void emptyArrayAndBlocks(char **array, size_t *arraySize) {
    if (array != NULL) {
        for (int i = 0; i < *arraySize; i++) {
            if (array[i] != NULL) {
                free(array[i]);
            }
        }
        free(array);
    }
}

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

void createTable(char **array, size_t *arraySize) {
    size_t size;
    int error = 0;
    size = readSizeT(&error);
    if (error < 0) {
        printf("Bad input");
        return;
    }
    if (size > 0) {
        if (array != NULL)emptyArrayAndBlocks(array, arraySize);
        if (createEmptyArray(array, arraySize, size) != 0) {
            printf("Not enough memory for a table\n");
            return;
        }
        printf("Table created\n");
    }
}

void searchDirectory(char **array, size_t *arraySize) {
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

void addToTable(char **array, size_t *arraySize) {
    const size_t nameFileTmpLength = 256;
    char fileTmp[nameFileTmpLength];
    scanf("%255s", fileTmp);
    if (fileTmp == NULL || strlen(fileTmp) == 0)
        return;
    addTemporaryFileBlockPointerToArray(array, arraySize, fileTmp);
}

int main() {
    size_t arraySize = 0;
    char **array = NULL;

    enum CMD currentCommand = NO_COMMAND;
    char cmd[255];
    while (currentCommand != EXIT) {
        printf("\n> ");
        scanf("%254s", cmd);
        currentCommand = NO_COMMAND;
        for (int i = 0; i < NUMBER_OF_COMMANDS; i++) {
            if (strcmp(cmd, commands[i]) == 0)currentCommand = i;
        }
        switch (currentCommand) {
            case CREATE_TABLE:
                createTable(array, &arraySize);
                break;
            case SEARCH_DIRECTORY:
                searchDirectory(array, &arraySize);
                break;
            case REMOVE_BLOCK:

                break;
            case ADD_TO_TABLE:
                addToTable(array, &arraySize);
                break;
            case EXIT:
                emptyArrayAndBlocks(array, &arraySize);
                break;
            default:
                printf("command %s not known", cmd);
        }
    }
    return 0;
}