#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <libgen.h>
#include <errno.h>

#define SINGLE_LINE_BUFOR_SIZE 256
#define DATE_FORMAT "_%F_%H-%M-%S"
#define LOG 1
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

enum COPY_TYPE {
    MEM,
    CP,
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

struct FILE_RECORD getFileRecord(char * line){
    size_t i=0;
    int deli=0;
    for(;i<strlen(line);i++){
        if(line[i]==';'){
            deli=1;
            break;
        }
    }
    if(deli==0){
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

int copyUsingMemory(char *fileName, struct timespec modificationTime) {
    my_headers("COPY LIB:\t");
    my_log("\n");

    my_log("\tOpening source file %s...\n", fileName);
    FILE *src = fopen(fileName, "r");
    if (src == NULL) {
        printf("copyUsingMemory: No file: %s\n", basename(fileName));
        exit(-1);
    }
    if (fseek(src, 0, SEEK_END) != 0) {
        printf("copyUsingMemory: file seek\n");
        exit(-1);
    }
    size_t size = ftell(src);
    if (fseek(src, 0, SEEK_SET) != 0) {
        printf("copyUsingMemory: file seek\n");
        exit(-1);
    }

    char destinationName[PATH_MAX];
    char date[30];
    strftime(date, 29, DATE_FORMAT, localtime(&modificationTime.tv_sec));
    strcpy(destinationName, "archiwum/");
    strcat(destinationName, basename(fileName));
    strcat(destinationName, date);

    my_log("\tOpening destination file %s...\n", destinationName);
    FILE *dest = fopen(destinationName, "w");
    if (dest == NULL) {
        printf("copyUsingMemory: Cannot create file: %s\n", destinationName);
        exit(-1);
    }

    my_log("\tCreating block...\n");
    char *block = malloc(size * sizeof(char));

    my_log("\tCopying block by block...\n");
    fread(block, sizeof(char), size, src);
    fwrite(block, sizeof(char), size, dest);


    my_log("\tClosing files...\n");
    fclose(src);
    fclose(dest);

    my_log("\tFreeing memory...\n");
    free(block);
    return 0;
}

int copyUsingCp(char *fileName, struct timespec modificationTime) {
    pid_t pid = fork();
    if (pid == 0) {
        char destinationName[PATH_MAX];
        char date[20];
        strftime(date, 19, DATE_FORMAT, localtime(&modificationTime.tv_sec));
        strcpy(destinationName, "archiwum/");
        strcat(destinationName, basename(fileName));
        strcat(destinationName, date);
        execlp("cp", "cp", fileName, destinationName, NULL);
    } else {
        int res;
        wait(&res);
    }
    return 0;
}

int monitor(char *fileName, time_t duration, time_t maxTime, enum COPY_TYPE type) {
    time_t startTime;
    time(&startTime);
    time_t currentTime;
    time(&currentTime);
    time_t lastCheck = 0;
    int modifications = 0;
    while (difftime(currentTime, startTime) < maxTime) {
        if (difftime(currentTime, lastCheck) > duration) {
            struct stat fileInfo;
            if (lstat(fileName, &fileInfo) == -1) {
                perror(fileName);
                exit(-1);
            }
            if (difftime(lastCheck, fileInfo.st_mtim.tv_sec) < 0) {
                switch (type) {
                    case CP:
                        copyUsingCp(fileName, fileInfo.st_mtim);
                        break;
                    case MEM:
                        copyUsingMemory(fileName, fileInfo.st_mtim);
                        break;
                }
                modifications++;
            }
            time(&lastCheck);
        }
        time(&currentTime);
    }
    return modifications;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Program expects at last 3 argument: [file name] [watch time] [type]\n");
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
    my_log("Analyzing file\n");

    struct FILES_ARRAY files_array = getFilesToWatchFromFile(argv[1]);

    struct MONITOR_RESULT results[files_array.size];

    pid_t pid;

    size_t i = 0;
    for (; i < files_array.size; i++) {
        pid = fork();
        if (pid == 0) {
            return monitor(files_array.files[i].dir, files_array.files[i].seconds, time, type);
        }
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