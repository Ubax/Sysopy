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

int arraySize = 0;

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

int addTemporaryFileBlockPointerToArray(char **blockArray) {
    int i = 0;
    for (; i < arraySize; i++) {
        if (blockArray[i] == NULL)break;
    }
    if (i == arraySize)
        if (!realloc(blockArray, ++arraySize))return -1;
    blockArray[i] = readTemporaryFile();
    return i;
}

char **createEmptyArray() {
    char **array = calloc(INITIAL_ARRAY_LENGTH, sizeof(char *));
    arraySize = INITIAL_ARRAY_LENGTH;
    for (int i = 0; i < arraySize; i++)array[i] = NULL;
    return array;
}

void deleteBlockFromArray(char **array, int i) {
    if (i >= arraySize || array[i] == NULL)return;
    free(array[i]);
    array[i] = NULL;
}

int main() {
    enum CMD currentCommand = NO_COMMAND;
    char * cmd;
    while(currentCommand!=EXIT){
        printf("\n> ");
        scanf("%s", cmd);
        for(int i=0;i<NUMBER_OF_COMMANDS;i++){
            if(strcmp(cmd, commands[i])==0)currentCommand=i;
        }

    }
    return 0;
}