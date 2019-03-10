#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define INITIAL_ARRAY_LENGTH 20
#define CONSOLE_LOG 0
#define ERROR_LOG 1

static inline void my_log(const char* msg, ...){
#if CONSOLE_LOG > 0
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
#endif
}

static inline void my_error(const char* msg, ...){
#if ERROR_LOG > 0
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
#endif
}


void findAndSaveResultToTemporaryFile(char *directory, char *fileName, char *tempFileName) {
    my_log("\texecuting find and saving result to temporary file...\n");
    char command[strlen(directory) + strlen(fileName) + 30];
    strcpy(command, "find ");
    strcat(command, directory);
    strcat(command, " -name \"");
    strcat(command, fileName);
    strcat(command, "\" > ");
    strcat(command, tempFileName);
    system(command);
    my_log("\tdone\n");
}

size_t fileSize(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

char *readTemporaryFile(char *fileName) {
    my_log("\treading temporary file...\n");
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        my_error("No temporary file with that name\n");
        return NULL;
    }

    my_log("\tcreating memory block...\n");
    char *block = (char *) calloc(fileSize(file), sizeof(char));

    char c;
    int i = 0;
    my_log("\tfilling memory block with data from file...\n");
    while ((c = fgetc(file)) != EOF) {
        block[i] = c;
        i++;
    }
    my_log("\tdone\n");
    return block;
}

int createEmptyArray(struct Array* array, size_t size) {
    my_log("\tcreating empty array...\n");
    my_log("\t\tsize: %lu\n", size);
    array->block = calloc(size, sizeof(char *));
    if (array->block == NULL){
        my_error("Couldn't create array in memory\n");
        return -1;
    }

    array->size = size;

    my_log("\tfilling array with NULLs...\n");
    for (int i = 0; i < array->size; i++)array->block[i] = NULL;

    my_log("\tdone\n");
    return 0;
}

int createEmptyDefaultSizeArray(struct Array* array) {
    my_log("\tcreating empty default size array...\n");
    return createEmptyArray(array, INITIAL_ARRAY_LENGTH);
}

int addTemporaryFileBlockPointerToArray(struct Array* array, char *tempFileName) {
    my_log("\tadding temporary file block pointer to array...\n");
    int i = 0;
    if (array->size < 1 || array->block == NULL){
        my_error("Array not created\n");
        return -1;
    }
    my_log("\tlooking for first empty cell...\n");
    for (; i < array->size; i++) {
        if (array->block[i] == NULL)break;
    }
    if (i == array->size){
        my_log("\tresizing array...\n");
        if (!realloc(array, ++array->size)){
            my_error("couldn't resize array\n");
            return -1;
        }
    }

    array->block[i] = readTemporaryFile(tempFileName);
    return i;
}

void deleteBlockFromArray(struct Array* array, int i) {
    my_log("\tdeleting block from array...\n");
    if (i >= array->size || array->block[i] == NULL){
        my_error("specified index cell empty\n");
        return;
    }
    my_log("\tfreeing array block...\n");
    free(array->block[i]);
    array->block[i] = NULL;
    my_log("\tdone\n");

}

void emptyArrayAndBlocks(struct Array* array) {
    my_log("\temptying array and memory blocks...\n");
    if (array->block != NULL) {
        my_log("\tfreeing memory blocks...\n");
        for (int i = 0; i < array->size; i++) {
            if (array->block[i] != NULL) {
                free(array->block[i]);
            }
        }
        free(array->block);
        my_log("\tdone\n");
    }
}
