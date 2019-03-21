#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define SINGLE_LINE_BUFOR_SIZE 256

struct FILE_RECORD {
    char dir[PATH_MAX];
    int seconds;
};

struct FILES_ARRAY {
    struct FILE_RECORD *files;
    size_t size;
};

size_t getNumberOfLines(char *fileName) {
    FILE *file = fopen(fileName, "r");
    char *line = NULL;
    size_t size = 0;
    size_t numberOfLines = 0;
    while (getline(&line, &size, file) != -1) {
        numberOfLines++;
        free(line);
    }
    return numberOfLines;
}

struct FILES_ARRAY getFilesToWatchFromFile(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("getFilesToWatchFromFile: No such file: %s\n", fileName);
    }
    struct FILES_ARRAY files_array;
    files_array.size = getNumberOfLines(fileName);
    files_array.files = malloc(sizeof(struct FILE_RECORD) * files_array.size);
    char *line = NULL;
    size_t size = 0;
    int i = 0;
    while (getline(&line, &size, file) != -1) {
        char *dir = strtok(line, ";");
        char *seconds = strtok(NULL, ";");
        strcpy(files_array.files[i].dir,dir);
        files_array.files[i].seconds = (int) strtol(seconds, NULL, 10);
        if (files_array.files[i].seconds <= 0) {
            printf("getFilesToWatchFromFile: Time should be positive\n");
            exit(1);
        }

        i++;
        free(line);
    }
    return files_array;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Program expects at 1 argument: [file name]\n");
        return 1;
    }
    struct FILES_ARRAY files_array = getFilesToWatchFromFile(argv[1]);
    size_t i = 0;
    for (; i < files_array.size; i++) {
        printf("Dir: %s\tTime: %i\n", files_array.files[i].dir, files_array.files[i].seconds);
    }
    return 0;
}