#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define LINE_MAX_LENGTH 128

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Program expects at last 1 argument: [fifo name]\n");
        return 1;
    }
    mkfifo(argv[1], S_IRUSR | S_IWUSR);
    char *bufor=malloc(LINE_MAX_LENGTH* sizeof(char));
    FILE* file = fopen(argv[1], "r");
    if(file==NULL){
        perror("FIFO OPENING");
        return 1;
    }
    size_t size = LINE_MAX_LENGTH;
    while (getline(&bufor, &size, file) != -1) {
        printf("%s", bufor);
    }
    free(bufor);

    return 0;
}