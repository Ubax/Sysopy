#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_ARRAY_LENGTH 20
#define CONSOLE_LOG 0
#define ERROR_LOG 1


void findAndSaveResultToTemporaryFile(char *directory, char *fileName, char *tempFileName) {
    if(CONSOLE_LOG)printf("\texecuting find and saving result to temporary file...\n");
    char command[strlen(directory) + strlen(fileName) + 30];
    strcpy(command, "find ");
    strcat(command, directory);
    strcat(command, " -name \"");
    strcat(command, fileName);
    strcat(command, "\" > ");
    strcat(command, tempFileName);
    system(command);
    if(CONSOLE_LOG)printf("\tdone\n");
}

size_t fileSize(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

char *readTemporaryFile(char *fileName) {
    if(CONSOLE_LOG)printf("\treading temporary file...\n");
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        if(ERROR_LOG)printf("No temporary file with that name\n");
        return NULL;
    }

    if(CONSOLE_LOG)printf("\tcreating memory block...\n");
    char *block = (char *) calloc(fileSize(file), sizeof(char));

    char c;
    int i = 0;
    if(CONSOLE_LOG)printf("\tfilling memory block with data from file...\n");
    while ((c = fgetc(file)) != EOF) {
        block[i] = c;
        i++;
    }
    if(CONSOLE_LOG)printf("\tdone\n");
    return block;
}

int createEmptyArray(struct Array* array, size_t size) {
    if(CONSOLE_LOG)printf("\tcreating empty array...\n");
    if(CONSOLE_LOG)printf("\t\tsize: %lu\n", size);
    array->block = calloc(size, sizeof(char *));
    if (array->block == NULL){
        if(ERROR_LOG)printf("Couldn't create array in memory\n");
        return -1;
    }

    array->size = size;

    if(CONSOLE_LOG)printf("\tfilling array with NULLs...\n");
    for (int i = 0; i < array->size; i++)array->block[i] = NULL;

    if(CONSOLE_LOG)printf("\tdone\n");
    return 0;
}

int createEmptyDefaultSizeArray(struct Array* array) {
    if(CONSOLE_LOG)printf("\tcreating empty default size array...\n");
    return createEmptyArray(array, INITIAL_ARRAY_LENGTH);
}

int addTemporaryFileBlockPointerToArray(struct Array* array, char *tempFileName) {
    if(CONSOLE_LOG)printf("\tadding temporary file block pointer to array...\n");
    int i = 0;
    if (array->size < 1 || array->block == NULL){
        if(ERROR_LOG)printf("Array not created\n");
        return -1;
    }
    if(CONSOLE_LOG)printf("\tlooking for first empty cell...\n");
    for (; i < array->size; i++) {
        if (array->block[i] == NULL)break;
    }
    if (i == array->size){
        if(CONSOLE_LOG)printf("\tresizing array...\n");
        if (!realloc(array, ++array->size)){
            if(ERROR_LOG)printf("couldn't resize array\n");
            return -1;
        }
    }

    array->block[i] = readTemporaryFile(tempFileName);
    return i;
}

void deleteBlockFromArray(struct Array* array, int i) {
    if(CONSOLE_LOG)printf("\tdeleting block from array...\n");
    if (i >= array->size || array->block[i] == NULL){
        if(ERROR_LOG)printf("specified index cell empty\n");
        return;
    }
    if(CONSOLE_LOG)printf("\tfreeing array block...\n");
    free(array->block[i]);
    array->block[i] = NULL;
    if(CONSOLE_LOG)printf("\tdone\n");

}

void emptyArrayAndBlocks(struct Array* array) {
    if(CONSOLE_LOG)printf("\temptying array and memory blocks...\n");
    if (array->block != NULL) {
        if(CONSOLE_LOG)printf("\tfreeing memory blocks...\n");
        for (int i = 0; i < array->size; i++) {
            if (array->block[i] != NULL) {
                free(array->block[i]);
            }
        }
        free(array->block);
        if(CONSOLE_LOG)printf("\tdone\n");
    }
}
