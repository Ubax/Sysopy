#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define SINGLE_LINE_BUFOR_SIZE 256

struct FILE_RECORD{
    char* dir;
    int seconds;
};

struct FILES_ARRAY{
    struct FILE_RECORD * files;
    size_t size;
};

char* readLine(FILE * file){
    if(file == NULL){
        printf("radLine: File is NULL\n");
        exit(1);
    }
    char* line = malloc(sizeof(char)*SINGLE_LINE_BUFOR_SIZE);
    if(line==NULL){
        printf("readLine: Cannot allocate line buffer\n");
        exit(1);
    }
    char c = getc(file);
    size_t i = 0;
    while (c!='\n'&&c!=EOF){
        if(i == SINGLE_LINE_BUFOR_SIZE){
            line = realloc(line, i+SINGLE_LINE_BUFOR_SIZE);
            if(line==NULL){
                printf("readLine: Cannot reallocate line buffer (%lu)\n", i);
                exit(1);
            }
        }
        i++;
        c=getc(file);
    }
}

struct FILES_ARRAY getFilesToWatchFromFile(char * fileName){

}

int main(int argc, char **argv){
    if (argc < 2) {
        printf("Program expects at 1 argument: [file name]\n");
        return 1;
    }

    return 0;
}