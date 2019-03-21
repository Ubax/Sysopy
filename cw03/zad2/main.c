#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define SINGLE_LINE_BUFOR_SIZE 256

struct FILE_RECORD {
    char dir[PATH_MAX];
    int seconds;
};

struct FILES_ARRAY {
    struct FILE_RECORD *files;
    size_t size;
};

struct MONITOR_RESULT {
    pid_t pid;
    int numberOfModifications;
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
        strcpy(files_array.files[i].dir, dir);
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

int monitor(char *fileName, time_t duration, time_t maxTime) {
    time_t startTime;
    time(&startTime);
    time_t currentTime;
    time(&currentTime);
    time_t lastCheck = 0;
    int modifications = 0;
    while (difftime(currentTime, startTime) < maxTime) {
        if (difftime(currentTime, lastCheck) > duration) {
            struct stat fileInfo;
            if (lstat(fileName, &fileInfo) == -1){
                perror(fileName);
                exit(-1);
            }
            if(difftime(lastCheck, fileInfo.st_mtim.tv_sec)<0){
                printf("%s - modification\n", fileName);
                modifications++;
            }
            time(&lastCheck);
        }
        time(&currentTime);
    }
    return modifications;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Program expects at 1 argument: [file name] [watch time]\n");
        return 1;
    }
    struct FILES_ARRAY files_array = getFilesToWatchFromFile(argv[1]);

    struct MONITOR_RESULT results[files_array.size];

    pid_t pid;

    size_t i = 0;
    for (; i < files_array.size; i++) {
        pid = fork();
        if (pid == 0) {
            return monitor(files_array.files[i].dir, files_array.files[i].seconds, 10);
        }
    }
    i = 0;
    for (; i < files_array.size; i++) {
        results[i].pid = wait(&results[i].numberOfModifications);
        results[i].numberOfModifications=WEXITSTATUS(results[i].numberOfModifications);
    }
    i=0;
    for (; i < files_array.size; i++) {
        if(results[i].numberOfModifications == -1)printf("PID: %i\tError\n", results[i].pid);
        else printf("PID: %i\tModifications: %i\n", results[i].pid, results[i].numberOfModifications);
    }
    return 0;
}