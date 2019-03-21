#define _XOPEN_SOURCE 500
#define LOG 0
#define HEADERS 0

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if LOG>0 || HEADERS>0
    #include <stdarg.h>
#endif
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <limits.h>


#ifndef SYSOPY_DIRREADER_H
#define SYSOPY_DIRREADER_H


enum ERRORS {
    NO_ERROR = 0,
    NO_DIRECTORY,
    CLOSING_DIRECTORY,
    READING_DIR,
    FILE_INFO,
};

enum TYPE {
    NO_TYPE,
    EARLIER,
    EXACT,
    LATER,
};

static inline void my_log(const char *msg, ...) {
#if LOG > 0
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
#endif
}

static inline void my_headers(const char *msg, ...) {
#if HEADERS > 0
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
#endif
}

enum ERRORS ors_analyze_files(DIR *dir, char *dirName);

enum ERRORS ors(char *dir);

void displayError(char *prefix, enum ERRORS error) {
    switch (error) {
        case NO_ERROR:
            break;
        case NO_DIRECTORY:
            printf("Error in %s: no directory\n", prefix);
            break;
        case CLOSING_DIRECTORY:
            printf("Error in %s: closing directory\n", prefix);
            break;
        case READING_DIR:
            printf("Error in %s: reading directory\n", prefix);
            break;
        case FILE_INFO:
            printf("Error in %s: file info %i\n", prefix, errno);
            break;
    }
}

int shouldAvoidDir(char *dirName) {
    if (strcmp(dirName, ".") == 0)return 1;
    else if (strcmp(dirName, "..") == 0)return 1;
    return 0;
}

enum ERRORS ors_analyze_files(DIR *dir, char *dirName) {
    my_log("\tReading files...\n");
    struct dirent *dirInfo;
    struct stat fileInfo;
    char newDirName[PATH_MAX];

    while ((dirInfo = readdir(dir)) != NULL) {
        if (shouldAvoidDir(dirInfo->d_name))continue;

        strcpy(newDirName, dirName);
        strcat(newDirName, dirInfo->d_name);

        if (lstat(newDirName, &fileInfo) == -1){
            perror(newDirName);
            return FILE_INFO;
        }

        if (S_ISDIR(fileInfo.st_mode) && !S_ISLNK(fileInfo.st_mode)) {
            strcat(newDirName, "/");
            enum ERRORS error = ors(newDirName);
            if (error != NO_ERROR)return error;
        }

    }
    return NO_ERROR;
}

enum ERRORS ors(char *dir) {
    my_headers("ORS\n");

    if (dir[strlen(dir) - 1] != '/')strcat(dir, "/");

    my_log("\tOpening directory...\n");
    DIR *directory;
    if ((directory = opendir(dir)) == NULL)return NO_DIRECTORY;

    pid_t pid = fork();
    if(pid==0){
        printf("Current dir: %s \tPID:%i\n", dir, getpid());
        execlp("ls","ls","-l", dir, NULL);
    }else{
        int status;
        wait(&status);
        if(status!=0){
            return status;
        }
    }

    enum ERRORS error = ors_analyze_files(directory, dir);
    if (error != NO_ERROR)return error;

    if (errno != 0){
        return READING_DIR;
    }

    my_log("\tClosing directory...\n");
    if (closedir(directory) != 0)return CLOSING_DIRECTORY;
    return NO_ERROR;
}

#endif //SYSOPY_DIRREADER_H
