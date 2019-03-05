#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_ARRAY_LENGTH 20

void findAndSaveResultToTemporaryFile(char *directory, char *fileName) {
    char command[strlen(directory)+strlen(fileName)+30];
    strcpy(command, "find ");
    strcat(command, directory);
    strcat(command, " -name \"");
    strcat(command, fileName);
    strcat(command, "\" > temp.txt");
    system(command);
}

size_t fileSize(FILE* f){
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

size_t arrayLength(void* array){
    return sizeof(array);
}

char* readTemporaryFile(){
    FILE* file = fopen("temp.txt", "r");
    if(file == NULL)return NULL;

    char * block = (char*)malloc(fileSize(file));

    char c;
    int i=0;
    while((c = fgetc(file)) != EOF){
        block[i]=c;
        i++;
    }
    return block;
}

int addTemporaryFileBlockPointerToArray(char** blockArray){
    int i=0;
    int array_length = sizeof(blockArray)/ sizeof(char*)
    for(;i<arrayLength(blockArray);i++){
        if(blockArray[i]==NULL)break;
    }
    if(i==arrayLength(blockArray))return -1;
        if(!realloc(blockArray, arrayLength(blockArray)+1))return -1;
    blockArray[i]=readTemporaryFile();
    return i;
}

char** createEmptyArray(){
    char** array = (char**)malloc(INITIAL_ARRAY_LENGTH);
    for(int i=0;i<INITIAL_ARRAY_LENGTH;i++)array[i]=NULL;
    return array;
}

int main() {
    findAndSaveResultToTemporaryFile("~/Documents/AGH/4-semestr/", "ui");
    printf("find");
    char ** array = createEmptyArray();
    printf("%lu",arrayLength(array));
    printf("after array");
    //printf("%i",addTemporaryFileBlockPointerToArray(array));
    return 0;
}