#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>
#include <sys/resource.h>
#include "monitor.h"

struct FILE_IN_MEMORY copyToMemory(char *fileName, struct timespec modificationTime) {
    my_log("\tOpening source file %s...\n", fileName);
    FILE *src = fopen(fileName, "r");
    if (src == NULL) {
        printf("copyToMemory: No file: %s\n", basename(fileName));
        exit(-1);
    }
    if (fseek(src, 0, SEEK_END) != 0) {
        printf("copyToMemory: file seek\n");
        exit(-1);
    }
    size_t size = ftell(src);
    if (fseek(src, 0, SEEK_SET) != 0) {
        printf("copyToMemory: file seek\n");
        exit(-1);
    }
    my_log("\tAllocating memory %s...\n", fileName);
    char *data = malloc(size * sizeof(char));
    if (data == NULL) {
        printf("copyToMemory: Couldn't allocate memory\n");
        exit(1);
    }
    fread(data, sizeof(char), size, src);
    fclose(src);

    struct FILE_IN_MEMORY fileInMemory;
    strcpy(fileInMemory.fileName, fileName);
    fileInMemory.modDate = modificationTime;
    fileInMemory.data = data;
    fileInMemory.size = size;

    return fileInMemory;
}

void pasteToArchive(struct FILE_IN_MEMORY *file) {
    if (file->data == NULL) {
        printf("pasteToArchive: Data is null\n");
        exit(1);
    }
    char destinationName[PATH_MAX];
    char date[30];
    strftime(date, 29, DATE_FORMAT, localtime(&file->modDate.tv_sec));
    strcpy(destinationName, "archiwum/");
    strcat(destinationName, basename(file->fileName));
    strcat(destinationName, date);

    my_log("\tOpening destination file %s...\n", destinationName);
    FILE *dest = fopen(destinationName, "w");
    if (dest == NULL) {
        printf("copyToMemory: Cannot create file: %s\n", destinationName);
        exit(-1);
    }
    my_log("\tWriting...\n");
    fwrite(file->data, sizeof(char), file->size, dest);


    my_log("\tClosing files...\n");
    fclose(dest);

    my_log("\tFreeing memory...\n");
    free(file->data);
    file->data = NULL;
    my_headers("File modified: %s\nSaved to: %s\n", file->fileName, destinationName);
}

int modifications = 0;

void signalINT(int signalno){
    printf("Odebrano sygnał SIGINT\n");
    exit(modifications);
}

int monitor(char *fileName, time_t duration, time_t maxTime) {
    time_t startTime;
    time(&startTime);
    time_t currentTime;
    time(&currentTime);
    time_t check_begin = 0;
    time_t lastArchive = 0;

    modifications = 0;
    signal(SIGINT, &signalINT);

    struct FILE_IN_MEMORY fileInMemory;
    fileInMemory.data = NULL;
    fileInMemory.modDate.tv_sec = 2;
    while (1) {
        time(&check_begin);
        struct stat fileInfo;
        if (lstat(fileName, &fileInfo) == -1) {
            perror(fileName);
            exit(-1);
        }
        my_headers("Checking file: %s\n", fileName);
        switch (type) {
            case CP:
                if (difftime(lastArchive, fileInfo.st_mtim.tv_sec) < 0) {
                    modifications++;
                    copyUsingCp(fileName, fileInfo.st_mtim);
                    time(&lastArchive);
                }
                break;
            case MEM:
                if (difftime(fileInfo.st_mtim.tv_sec, fileInMemory.modDate.tv_sec) > 0) {
                    modifications++;
                    my_headers("File modified: %s\n", fileName);
                    if (fileInMemory.data != NULL)pasteToArchive(&fileInMemory);
                    fileInMemory = copyToMemory(fileName, fileInfo.st_mtim);
                }
                break;
        }
        time(&currentTime);
        sleep((unsigned) (duration - difftime(currentTime, check_begin)));
    }
}