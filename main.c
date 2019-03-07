#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_ARRAY_LENGTH 20
#define NUMBER_OF_COMMANDS 4

const char *commands[NUMBER_OF_COMMANDS] = {
        "create_table",
        "search_directory",
        "remove_block",
        "exit"
};

enum CMD {
    NO_COMMAND = -1,
    CREATE_TABLE = 0,
    SEARCH_DIRECTORY = 1,
    REMOVE_BLOCK = 2,
    EXIT = 3,
};

void findAndSaveResultToTemporaryFile(char *directory, char *fileName) {
    char command[strlen(directory) + strlen(fileName) + 30];
    strcpy(command, "find ");
    strcat(command, directory);
    strcat(command, " -name \"");
    strcat(command, fileName);
    strcat(command, "\" > temp.txt");
    system(command);
}

size_t fileSize(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

/*size_t arrayLength(void* array){
    return sizeof(array);
}*/

char *readTemporaryFile() {
    FILE *file = fopen("temp.txt", "r");
    if (file == NULL)return NULL;

    char *block = (char *) calloc(fileSize(file), sizeof(char));

    char c;
    int i = 0;
    while ((c = fgetc(file)) != EOF) {
        block[i] = c;
        i++;
    }
    return block;
}

int addTemporaryFileBlockPointerToArray(char **array, size_t * arraySize) {
    int i = 0;
    for (; i < (*arraySize); i++) {
        if (array[i] == NULL)break;
    }
    if (i == (*arraySize))
        if (!realloc(array, ++(*arraySize)))return -1;
    array[i] = readTemporaryFile();
    return i;
}

int createEmptyArray(char** array, size_t * arraySize, size_t size) {
    array = calloc(size, sizeof(char *));
    if(array==NULL)return -1;
    for (int i = 0; i < size; i++)array[i] = NULL;
    (*arraySize) = size;
    return 0;
}

int createEmptyDefaultSizeArray(char** array, size_t * arraySize) {
    return createEmptyArray(array, arraySize, INITIAL_ARRAY_LENGTH);
}

void deleteBlockFromArray(char **array, size_t * arraySize, int i) {
    if (i >= *arraySize || array[i] == NULL)return;
    free(array[i]);
    array[i] = NULL;
}

void emptyArrayAndBlocks(char ** array, size_t * arraySize){
    if(array != NULL){
        for(int i=0;i<*arraySize;i++){
            if(array[i]!=NULL){
                free(array[i]);
            }
        }
        free(array);
    }
}

void createTable(char ** array, size_t * arraySize){
    size_t size;
    scanf("%zu", &size);
    if(size > 0){
        if(array != NULL)emptyArrayAndBlocks(array, arraySize);
        if(createEmptyArray(array, arraySize, size)!=0){
            printf("Not enough memory for a table\n");
            return;
        }
        printf("Table created\n");
    }
}

int main() {
    size_t arraySize = 0;
    char ** array = NULL;

    enum CMD currentCommand = NO_COMMAND;
    char cmd[255];
    while(currentCommand!=EXIT){
        printf("\n> ");
        scanf("%254s", cmd);
        currentCommand=NO_COMMAND;
        for(int i=0;i<NUMBER_OF_COMMANDS;i++){
            if(strcmp(cmd, commands[i])==0)currentCommand=i;
        }
        switch (currentCommand){
            case CREATE_TABLE:
                createTable(array, &arraySize);
                break;
            case SEARCH_DIRECTORY:

                break;
            case REMOVE_BLOCK:

                break;
            case EXIT:
                break;
            default:
                printf("command %s not known",cmd);
        }
    }
    return 0;
}