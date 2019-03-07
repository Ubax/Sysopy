#ifndef ZAD1LIB_LIBRARY_H
#define ZAD1LIB_LIBRARY_H

#include <stdio.h>

struct Array{
    char** block;
    size_t size;
};

size_t fileSize(FILE *f);
void findAndSaveResultToTemporaryFile(char *directory, char *fileName, char *tempFileName);
char *readTemporaryFile(char *fileName);
int createEmptyDefaultSizeArray(struct Array* array);
int addTemporaryFileBlockPointerToArray(struct Array* array, char *tempFileName);
void deleteBlockFromArray(struct Array* array, int i);
void emptyArrayAndBlocks(struct Array* array);

#endif