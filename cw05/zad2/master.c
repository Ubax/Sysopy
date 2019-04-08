#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LENGTH 128

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Program expects at last 1 argument: [fifo name]\n");
        return 1;
    }
    if(mkfifo(argv[1], S_IRUSR | S_IWUSR)<0){
        perror("FIFO creating");
        return 1;
    }
    char *bufor=malloc(LINE_MAX_LENGTH * sizeof(char));
    FILE* file = fopen(argv[1], "r");
    if(!file){
        perror("FIFO OPENING");
        return 1;
    }
    while (fgets(bufor, LINE_MAX_LENGTH, file)!=NULL) {
        printf("%s", bufor);
    }
    free(bufor);
    fclose(file);
    return 0;
}