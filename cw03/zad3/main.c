#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/resource.h>
#include "monitor.h"

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

struct FILE_RECORD getFileRecord(char *line) {
    size_t i = 0;
    int deli = 0;
    for (; i < strlen(line); i++) {
        if (line[i] == ';') {
            deli = 1;
            break;
        }
    }
    if (deli == 0) {
        printf("No delimeter [;]\n");
        exit(1);
    }
    struct FILE_RECORD fr;
    char *dir = strtok(line, ";");
    char *seconds = strtok(NULL, ";");
    strcpy(fr.dir, dir);
    fr.seconds = (int) strtol(seconds, NULL, 10);
    if (fr.seconds <= 0) {
        printf("getFilesToWatchFromFile: Time should be positive\n");
        exit(1);
    }
    return fr;
}

struct FILES_ARRAY getFilesToWatchFromFile(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("getFilesToWatchFromFile: No such file: %s\n", fileName);
        exit(1);
    }
    struct FILES_ARRAY files_array;
    files_array.size = getNumberOfLines(fileName);
    files_array.files = malloc(sizeof(struct FILE_RECORD) * files_array.size);
    char *line = NULL;
    size_t size = 0;
    int i = 0;
    while (getline(&line, &size, file) != -1) {
        files_array.files[i] = getFileRecord(line);
        i++;
        free(line);
    }
    return files_array;
}

int main(int argc, char **argv) {
    if (argc < 6) {
        printf("Program expects at last 5 argument: [file name] [watch time] [type] [cpu time limit] [memory limit]\n");
        return 1;
    }
    enum COPY_TYPE type;
    if (strcmp("MEM", argv[3]) == 0 || strcmp("mem", argv[3]) == 0)type = MEM;
    else if (strcmp("CP", argv[3]) == 0 || strcmp("cp", argv[3]) == 0)type = CP;
    else {
        printf("Type should be either MEM or CP\n");
        return 1;
    }

    struct stat sb;

    if (!(stat("archiwum", &sb) == 0 && S_ISDIR(sb.st_mode))) {
        errno = 0;
        printf("Creating directory: archiwum\n");
        mkdir("archiwum", ACCESSPERMS);
        if (errno != 0) {
            perror("ref");
            printf("Error creating directory: archiwum\nCreate it manually or try again\n");
        }
    }

    int time = (int) strtol(argv[2], NULL, 10);
    if (time <= 0) {
        printf("Time should be positive\n");
        exit(1);
    }

    rlim_t cpuTimeLimit = (rlim_t) strtol(argv[4], NULL, 10);
    if (cpuTimeLimit <= 0) {
        printf("CPU Time limit should be positive\n");
        exit(1);
    }

    rlim_t memoryLimit = (rlim_t) strtol(argv[5], NULL, 10);
    if (memoryLimit <= 0) {
        printf("Memory limit should be positive\n");
        exit(1);
    }

    struct rlimit cpuLimit;
    cpuLimit.rlim_max = cpuTimeLimit;
    cpuLimit.rlim_cur = cpuTimeLimit;
    struct rlimit memLimit;
    memLimit.rlim_max = memoryLimit * 1024 * 1024;
    memLimit.rlim_cur = memoryLimit * 1024 * 1024;


    my_log("Analyzing file\n");

    struct FILES_ARRAY files_array = getFilesToWatchFromFile(argv[1]);

    struct MONITOR_RESULT results[files_array.size];

    pid_t pid;

    size_t i = 0;
    for (; i < files_array.size; i++) {
        pid = fork();
        if (pid == 0) {
            setrlimit(RLIMIT_AS, &memLimit);
            setrlimit(RLIMIT_CPU, &cpuLimit);
            if (errno) {
                perror("Setting limits");
                exit(-2);
            }
            return monitor(files_array.files[i].dir, files_array.files[i].seconds, time, type);
        }
    }


    i = 0;
    for (; i < files_array.size; i++) {
        struct rusage resourcesUsageBeg;
        struct rusage resourcesUsage;
        if (getrusage(RUSAGE_CHILDREN, &resourcesUsageBeg)) {
            perror("Resources before watch");
            exit(1);
        }
        results[i].pid = wait(&results[i].numberOfModifications);
        results[i].numberOfModifications = WEXITSTATUS(results[i].numberOfModifications);
        if (getrusage(RUSAGE_CHILDREN, &resourcesUsage)) {
            perror("Resources after watch");
            exit(1);
        }
        if (results[i].numberOfModifications == -1)printf("PID: %i\tError\n", results[i].pid);
        else {
            printf("PID: %i\tModifications: %i\tUser time: %lu.%08ld\tSystem time: %lu.%08ld\n", results[i].pid,
                   results[i].numberOfModifications,
                   resourcesUsage.ru_utime.tv_sec-resourcesUsageBeg.ru_utime.tv_sec,
                   resourcesUsage.ru_utime.tv_usec-resourcesUsageBeg.ru_utime.tv_usec,
                   resourcesUsage.ru_stime.tv_sec-resourcesUsageBeg.ru_stime.tv_sec,
                   resourcesUsage.ru_stime.tv_usec-resourcesUsageBeg.ru_stime.tv_usec);
        }
    }
    return 0;
}