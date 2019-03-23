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
    char * data = malloc(size * sizeof(char));
    if (data == NULL) {
        printf("copyToMemory: Couldn't allocate memory\n");
        exit(1);
    }
    fread(data, sizeof(char), size, src);
    fclose(src);

    struct FILE_IN_MEMORY fileInMemory;
    strcpy(fileInMemory.fileName,fileName);
    fileInMemory.modDate=modificationTime;
    fileInMemory.data=data;
    fileInMemory.size=size;

    return fileInMemory;
}

void pasteToArchive(struct FILE_IN_MEMORY * file) {
    if(file->data==NULL) {
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
    file->data=NULL;
    my_headers("File modified: %s\nSaved to: %s\n", file->fileName, destinationName);
}

int copyUsingCp(char *fileName, struct timespec modificationTime) {
    pid_t pid = fork();
    if (pid == 0) {
        char destinationName[PATH_MAX];
        char date[30];
        strftime(date, 29, DATE_FORMAT, localtime(&modificationTime.tv_sec));
        strcpy(destinationName, "archiwum/");
        strcat(destinationName, basename(fileName));
        strcat(destinationName, date);
        my_headers("File modified: %s\nSaved to: %s\n", fileName, destinationName);
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
    time_t check_begin = 0;
    time_t lastCheck = 0;
    int modifications = 0;
    struct FILE_IN_MEMORY fileInMemory;
    fileInMemory.data=NULL;
    fileInMemory.modDate.tv_sec=2;
    while (difftime(currentTime, startTime) < maxTime) {
        time(&check_begin);
        struct stat fileInfo;
        if (lstat(fileName, &fileInfo) == -1) {
            perror(fileName);
            exit(-1);
        }
        my_headers("Checking file: %s\n", fileName);
        switch (type) {
            case CP:
                if (difftime(lastCheck, fileInfo.st_mtim.tv_sec) < 0) {
                    modifications++;
                    copyUsingCp(fileName, fileInfo.st_mtim);
                }
                break;
            case MEM:
                if (difftime(fileInfo.st_mtim.tv_sec, fileInMemory.modDate.tv_sec) > 0) {
                    modifications++;
                    my_headers("File modified: %s\n", fileName);
                    if(fileInMemory.data!=NULL)pasteToArchive(&fileInMemory);
                    fileInMemory=copyToMemory(fileName, fileInfo.st_mtim);
                }
                break;
        }
        time(&currentTime);
        time(&lastCheck);
        sleep((unsigned)(duration-difftime(currentTime, check_begin)));
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