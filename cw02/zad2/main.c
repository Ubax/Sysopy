#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

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

enum ERRORS {
    NO_ERROR = 0,
    NO_DIRECTORY,
    CLOSING_DIRECTORY,
    READING_DIR,
    FILE_INFO,
};

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

int comp(time_t time1, time_t time2, char *type) {
    if (strcmp(type, ">") == 0)return time2 > time1;
    if (strcmp(type, "=") == 0)return time2 == time1;
    if (strcmp(type, "<") == 0)return time2 < time1;
    return 0;
}

enum ERRORS ors(char *dir, char *type, char *date) {
    my_headers("ORS\n");

    if (dir[strlen(dir) - 1] != '/')strcat(dir, "/");

    my_log("\tOpening directory...\n");
    DIR *directory;
    if ((directory = opendir(dir)) == NULL)return NO_DIRECTORY;

    my_log("\tReading files...\n");
    struct dirent *dirInfo;
    struct stat fileInfo;
    char dirName[4096];
    while ((dirInfo = readdir(directory)) != NULL) {
        if (strcmp(dirInfo->d_name, ".") == 0)continue;
        else if (strcmp(dirInfo->d_name, "..") == 0)continue;

        strcpy(dirName, dir);
        strcat(dirName, dirInfo->d_name);
        my_log("%s\n", dirName);
        if (lstat(dirName, &fileInfo) == -1)return FILE_INFO;

        if (S_ISDIR(fileInfo.st_mode) && !S_ISLNK(fileInfo.st_mode)) {
            strcat(dirName, "/");
            enum ERRORS error = ors(dirName, type, date);
            if (error != NO_ERROR)return error;
        } else {
            if (comp(time(0) - 1000, fileInfo.st_mtim.tv_sec, type)) {
                char date[256];
                strftime(date, 255, "%F", localtime(&fileInfo.st_mtim.tv_sec));
                printf("%s\t%s\n", dirInfo->d_name, date);
            }
        }
    }

    if (errno != 0)return READING_DIR;

    my_log("\tClosing directory...\n");
    if (closedir(directory) != 0)return CLOSING_DIRECTORY;
    return NO_ERROR;
}

enum ERRORS n(char *dir, char *type, char *date) {
    return NO_ERROR;
}


int ors_preprepare(char *dir, char *type, char *date) { // opendir, readdir, stat
    enum ERRORS error = ors(dir, type, date);
    displayError("ORS", error);
    if (error != NO_ERROR)return 1;
    return 0;
}

int n_preprepaer(char *dir, char *type, char *date) { //nftw
    enum ERRORS error = n(dir, type, date);
    displayError("NFTW", error);
    if (error != NO_ERROR)return 1;
    return 0;
};

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Program expects at 3 arguments: [directory] [type (<,=,>)] [date]\n");
        return 1;
    }
    if (strcmp(argv[2], "<") != 0 && strcmp(argv[2], "=") != 0 && strcmp(argv[2], ">") != 0) {
        printf("Second argument should be one of < = >\n");
        return 1;
    }

    if (ors_preprepare(argv[1], argv[2], argv[3]) != 0)return 1;

    if (n_preprepaer(argv[1], argv[2], argv[3]) != 0)return 1;
    return 0;
}

