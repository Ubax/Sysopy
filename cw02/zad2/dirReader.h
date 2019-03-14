#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pwd.h>
#define _XOPEN_SOURCE 500
#include <ftw.h>

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

#define LOG 0
#define HEADERS 0

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

struct tm gtm;
enum TYPE gtype;

enum ERRORS ors_analyze_files(DIR *dir, enum TYPE type, char *dirName, struct tm date);

enum ERRORS ors(char *dir, enum TYPE type, struct tm date);

enum ERRORS n(char *dir, enum TYPE type, struct tm date);

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

int comp(time_t base, time_t time1, enum TYPE type) {
    base += 60 * 60;
    switch (type) {
        case EARLIER:
            return difftime(base, time1) < 0;
        case EXACT:
            return difftime(base, time1) == 0;
        case LATER:
            return difftime(base, time1) > 0;
        case NO_TYPE:
            return 0;
    }
    return 0;
}

int shouldAvoidDir(char *dirName) {
    if (strcmp(dirName, ".") == 0)return 1;
    else if (strcmp(dirName, "..") == 0)return 1;
    return 0;
}

enum ERRORS ors_display(char *fullDir, struct stat fileInfo, enum TYPE type, struct tm date) {
    //File Name			Mod Date	Acc Date	Type	Size
    if (comp(mktime(&date), fileInfo.st_mtime, type)) {
        char modDate[256];
        strftime(modDate, 255, "%F,%T", localtime(&fileInfo.st_mtime));
        char accDate[256];
        strftime(accDate, 255, "%F,%T", localtime(&fileInfo.st_atime));

        char *fType = "unknown";
        if (S_ISREG(fileInfo.st_mode))fType = "file";
        else if (S_ISDIR(fileInfo.st_mode))fType = "dir";
        else if (S_ISCHR(fileInfo.st_mode))fType = "char dev";
        else if (S_ISBLK(fileInfo.st_mode))fType = "block dev";
        else if (S_ISFIFO(fileInfo.st_mode))fType = "fifo";
        else if (S_ISLNK(fileInfo.st_mode))fType = "slink";
        else if (S_ISSOCK(fileInfo.st_mode))fType = "sock";

        printf("%s\t%s\t%s\t%i\t%s\n", fullDir, modDate, accDate, (int) fileInfo.st_size, fType);
    }
    return NO_ERROR;
}

enum ERRORS ors_analyze_files(DIR *dir, enum TYPE type, char *dirName, struct tm date) {
    my_log("\tReading files...\n");
    struct dirent *dirInfo;
    struct stat fileInfo;
    char newDirName[4096];

    while ((dirInfo = readdir(dir)) != NULL) {
        if (shouldAvoidDir(dirInfo->d_name))continue;

        strcpy(newDirName, dirName);
        strcat(newDirName, dirInfo->d_name);

        if (lstat(newDirName, &fileInfo) == -1)return FILE_INFO;

        if (S_ISDIR(fileInfo.st_mode) && !S_ISLNK(fileInfo.st_mode)) {
            strcat(newDirName, "/");
            enum ERRORS error = ors(newDirName, type, date);
            if (error != NO_ERROR)return error;
        } else {
            enum ERRORS error = ors_display(newDirName, fileInfo, type, date);
            if (error != NO_ERROR)return error;
        }
    }
    return NO_ERROR;
}

enum ERRORS ors(char *dir, enum TYPE type, struct tm date) {
    my_headers("ORS\n");

    if (dir[strlen(dir) - 1] != '/')strcat(dir, "/");

    my_log("\tOpening directory...\n");
    DIR *directory;
    if ((directory = opendir(dir)) == NULL)return NO_DIRECTORY;


    enum ERRORS error = ors_analyze_files(directory, type, dir, date);
    if (error != NO_ERROR)return error;

    if (errno != 0)return READING_DIR;

    my_log("\tClosing directory...\n");
    if (closedir(directory) != 0)return CLOSING_DIRECTORY;
    return NO_ERROR;
}

int nftw_display(char *fullDir, struct stat *fileInfo, int fd, struct FTW *flags) {
    //File Name			Mod Date	Acc Date	Type	Size
    if (comp(mktime(&gtm), fileInfo->st_mtime, gtype)) {
        char modDate[256];
        strftime(modDate, 255, "%F,%T", localtime(&fileInfo->st_mtime));
        char accDate[256];
        strftime(accDate, 255, "%F,%T", localtime(&fileInfo->st_atime));

        char *fType = "unknown";
        if (S_ISREG(fileInfo->st_mode))fType = "file";
        else if (S_ISDIR(fileInfo->st_mode))fType = "dir";
        else if (S_ISCHR(fileInfo->st_mode))fType = "char dev";
        else if (S_ISBLK(fileInfo->st_mode))fType = "block dev";
        else if (S_ISFIFO(fileInfo->st_mode))fType = "fifo";
        else if (S_ISLNK(fileInfo->st_mode))fType = "slink";
        else if (S_ISSOCK(fileInfo->st_mode))fType = "sock";

        printf("%s\t%s\t%s\t%i\t%s\n", fullDir, modDate, accDate, (int) fileInfo.st_size, fType);
    }
}

enum ERRORS n(char *dir, enum TYPE type, struct tm date) {
    nftw(dir, nftw_display, 10, FTW_PHYS);
    return NO_ERROR;
}

#endif //SYSOPY_DIRREADER_H
