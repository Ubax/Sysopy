#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/types.h>

#define STOP_INTTERUPT SIGUSR1
#define START_INTTERUPT SIGUSR2

#ifndef SYSOPY_MONITOR_H
#define SYSOPY_MONITOR_H

#define DATE_FORMAT "_%F_%H-%M-%S"
#define LOG 0
#define HEADERS 1

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

struct FILE_IN_MEMORY {
    char *data;
    size_t size;

    char fileName[PATH_MAX];
    struct timespec modDate;
};


int monitor(char *fileName, time_t duration);
struct FILE_IN_MEMORY copyToMemory(char *fileName, struct timespec modificationTime);
void pasteToArchive(struct FILE_IN_MEMORY *file);


#endif //SYSOPY_MONITOR_H
