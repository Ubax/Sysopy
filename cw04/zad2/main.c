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
#include "monitor.h"
#include "fileAnalyzer.h"

enum CMD{
    END,
    LIST,
    STOP_PID,
    START_PID,
    STOP_ALL,
    START_ALL,
    NONE,
};

enum CMD readCMD();

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

    struct MONITOR_RESULT results[files_array.size];

    pid_t pid[files_array.size];

    size_t i = 0;
    for (; i < files_array.size; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            return monitor(files_array.files[i].dir, files_array.files[i].seconds, time, type);
        }
    }

    enum CMD currentCMD = NONE;
    while(currentCMD!=END){
        currentCMD = readCMD();
    }
    i = 0;
    for (; i < files_array.size; i++) {
        results[i].pid = wait(&results[i].numberOfModifications);
        results[i].numberOfModifications = WEXITSTATUS(results[i].numberOfModifications);
    }
    i = 0;
    for (; i < files_array.size; i++) {
        if (results[i].numberOfModifications == -1)printf("PID: %i\tError\n", results[i].pid);
        else printf("PID: %i\tModifications: %i\n", results[i].pid, results[i].numberOfModifications);
    }
    return 0;
}

enum CMD readCMD(){
    char buf[255];
    scanf("%s", buf);
    if(strcmp(buf, "END")==0)return END;
    return NONE;
}