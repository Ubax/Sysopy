#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_ARRAY_LENGTH 20

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