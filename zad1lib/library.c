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
    if (file == NULL) {
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

int createEmptyArray(struct Array* array, size_t size) {
    array->block = calloc(size, sizeof(char *));
    if (array->block == NULL)return -1;

    array->size = size;

    for (int i = 0; i < array->size; i++)array->block[i] = NULL;

    return 0;
}

int createEmptyDefaultSizeArray(struct Array* array) {
    return createEmptyArray(array, INITIAL_ARRAY_LENGTH);
}

int addTemporaryFileBlockPointerToArray(struct Array* array, char *tempFileName) {
    int i = 0;
    if (array->size < 1 || array->block == NULL)return -1;
    for (; i < array->size; i++) {
        if (array->block[i] == NULL)break;
    }
    if (i == array->size)
        if (!realloc(array, ++array->size))return -1;
    array->block[i] = readTemporaryFile(tempFileName);
    return i;
}

void deleteBlockFromArray(struct Array* array, int i) {
    if (i >= array->size || array->block[i] == NULL)return;
    free(array->block[i]);
    array->block[i] = NULL;
}

void emptyArrayAndBlocks(struct Array* array) {
    if (array->block != NULL) {
        for (int i = 0; i < array->size; i++) {
            if (array->block[i] != NULL) {
                free(array->block[i]);
            }
        }
        free(array->block);
    }
}
