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
#include <ctype.h>
#include "monitor.h"
#include "fileAnalyzer.h"
#include "argsProcessor.h"


struct PID_FILE_RECORD {
    char *fileName;
    pid_t pid;
};

enum CMD {
    END,
    LIST,
    STOP_PID,
    START_PID,
    STOP_ALL,
    START_ALL,
    NONE,
};

enum CMD readCMD();

void processCMD(enum CMD, struct PID_FILE_RECORD *, size_t);
void end(int signalNumber);

int cmdPID = 0;

struct PID_FILE_RECORD * global_pid;
size_t global_arraySize;

/******** TO DO CTRL+C exit ********/


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Program expects at last 1 argument: [file name]\n");
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

    my_log("Analyzing file\n");

    struct FILES_ARRAY files_array = getFilesToWatchFromFile(argv[1]);

    struct PID_FILE_RECORD pid[files_array.size];
    global_pid=pid;
    global_arraySize=files_array.size;

    size_t i = 0;
    for (; i < files_array.size; i++) {
        pid[i].fileName = files_array.files[i].dir;
        pid[i].pid = fork();
        if (pid[i].pid == 0) {
            return monitor(files_array.files[i].dir, files_array.files[i].seconds);
        }
    }

    signal(SIGINT, end);

    enum CMD currentCMD = NONE;
    while (currentCMD != END) {
        currentCMD = readCMD();
        processCMD(currentCMD, pid, files_array.size);
    }
    end(0);
    return 0;
}

void end(int signalNumber){
    size_t i = 0;
    struct MONITOR_RESULT results[global_arraySize];
    for (; i < global_arraySize; i++) {
        kill(global_pid[i].pid, SIGINT);
        results[i].pid = wait(&results[i].numberOfModifications);
        results[i].numberOfModifications = WEXITSTATUS(results[i].numberOfModifications);
    }
    i = 0;
    for (; i < global_arraySize; i++) {
        if (results[i].numberOfModifications == 255)printf("PID: %i\tError\n", results[i].pid);
        else printf("PID: %i\tModifications: %i\n", results[i].pid, results[i].numberOfModifications);
    }
    exit(0);
}

void list(struct PID_FILE_RECORD *pidArray, size_t arraySize) {
    size_t i = 0;
    printf("\n|---------|----------------------------|\n"
           "|   PID   |          FILE NAME         |\n"
           "|---------|----------------------------|\n");
    for (; i < arraySize; i++)printf("| %7i | %26s |\n", pidArray[i].pid, pidArray[i].fileName);
    printf("|---------|----------------------------|\n\n");
}

void stopPID(pid_t pid) {
    if (kill(pid, STOP_INTTERUPT) != 0) {
        printf("Couldn't send a signal\n");
        perror("stop pid");
        errno = 0;
        return;
    }
    printf("Process %i stopped\n", pid);
}

void stopAll(struct PID_FILE_RECORD *pidArray, size_t arraySize) {
    size_t i = 0;
    for (; i < arraySize; i++) {
        stopPID(pidArray[i].pid);
    }
}

void startPID(pid_t pid) {
    if (kill(pid, START_INTTERUPT) != 0) {
        printf("Couldn't send a signal\n");
        perror("start pid");
        errno = 0;
        return;
    }
    printf("Process %i started\n", pid);
}

void startAll(struct PID_FILE_RECORD *pidArray, size_t arraySize) {
    size_t i = 0;
    for (; i < arraySize; i++) {
        startPID(pidArray[i].pid);
    }
}

void processCMD(enum CMD cmd, struct PID_FILE_RECORD *pidArray, size_t arraySize) {
    switch (cmd) {
        case LIST:
            list(pidArray, arraySize);
            break;
        case START_PID:
            startPID(cmdPID);
            break;
        case STOP_PID:
            stopPID(cmdPID);
            break;
        case START_ALL:
            startAll(pidArray, arraySize);
            break;
        case STOP_ALL:
            stopAll(pidArray, arraySize);
            break;
        case END:
            break;
        case NONE:
            printf("Unknown command\n");
            break;
    }
}

enum CMD readCMD() {
    char buf[255];
    scanf("%s", buf);
    toUpper(buf);
    if (strcmp(buf, "END") == 0)return END;
    if (strcmp(buf, "LIST") == 0)return LIST;
    if (strcmp(buf, "START") == 0) {
        scanf("%s", buf);
        toUpper(buf);
        if (strcmp(buf, "PID") == 0) {
            char buf[25];
            scanf("%s", buf);
            cmdPID=(int) strtol(buf, NULL, 10);
            if(errno || cmdPID<10){
                if(errno)perror("Invalid command argument");
                else printf("Invalid command argument\n");
                return NONE;
            }
            return START_PID;
        }
        if (strcmp(buf, "ALL") == 0)return START_ALL;
    }
    if (strcmp(buf, "STOP") == 0) {
        scanf("%s", buf);
        toUpper(buf);
        if (strcmp(buf, "PID") == 0) {
            char buf[25];
            scanf("%s", buf);
            cmdPID=(int) strtol(buf, NULL, 10);
            if(errno  || cmdPID<10){
                if(errno)perror("Invalid command argument");
                else printf("Invalid command argument\n");
                return NONE;
            }
            return STOP_PID;
        }
        if (strcmp(buf, "ALL") == 0)return STOP_ALL;
    }
    return NONE;
}