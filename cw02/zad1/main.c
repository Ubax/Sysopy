#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>

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

enum TYPE {
    NO_TYPE = -1,
    LIB = 0,
    SYS = 1,
};

enum ERRORS {
    UNKNOWN_TYPE = -1,
    NO_ERROR = 0,
    FILE_OPENING = 1,
    MEMORY_ALLOCATION = 2,
    FILE_WRITING = 3,
    FILE_READING = 4,
    FILE_SIZE = 5,
    FILE_SEEK = 6,
};

enum ERRORS generate(char *fileName, size_t numberOfRecords, size_t sizeOfBlock) {
    my_headers("GENERATE:\t");
    my_log("\n");

    my_log("\tOpening file %s...\n", fileName);
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        printf("Problem while opening file\n");
        return FILE_OPENING;
    }

    my_log("\tSetting seed to time\n");
    srand((unsigned int) time(NULL));
    char *block;
    size_t i = 0;
    for (; i < numberOfRecords; i++) {
        if ((block = malloc(sizeof(char) * sizeOfBlock)) == NULL) {
            printf("Problem with memory while creating block\n");
            return MEMORY_ALLOCATION;
        }
        size_t j = 0;
        for (; j < sizeOfBlock; j++) {
            block[j] = (char) (rand() % 26 + 'A');
        }
        fwrite(block, sizeof(char), sizeOfBlock, file);
        if (errno != 0) {
            printf("Error occured while writing to file\n");
            return FILE_WRITING;
        }
        free(block);
    }

    my_log("\tClosing file...\n");
    fclose(file);
    return NO_ERROR;
}

enum ERRORS sort_lib(char *fileName, size_t numberOfRecords, size_t sizeOfBlock) {
    my_headers("SORT LIB:\t");
    my_log("\n");

    my_log("\tOpening file %s...\n", fileName);
    FILE *file = fopen(fileName, "r+");
    if (file == NULL) {
        return FILE_OPENING;
    }

    my_log("\tChecking file size...\n");
    if (fseek(file, 0, SEEK_END) != 0)return FILE_SEEK;
    if (ftell(file) < numberOfRecords * sizeOfBlock) {
        return FILE_SIZE;
    }
    if (fseek(file, 0, SEEK_SET) != 0)return FILE_SEEK;

    my_log("\tCreating blocks...\n");
    char *block1 = malloc(sizeof(char) * (sizeOfBlock + 1));
    char *block2 = malloc(sizeof(char) * (sizeOfBlock + 1));

    my_log("\tSelection sort...\n");
    size_t i = 0;
    for (; i < numberOfRecords; i++) {
        size_t index = i;
        size_t j = i;
        if (fseek(file, i * sizeOfBlock, SEEK_SET) != 0)return FILE_SEEK;
        char c = (char) fgetc(file);
        char cur;
        for (; j < numberOfRecords; j++) {
            if (fseek(file, j * sizeOfBlock, SEEK_SET) != 0)return FILE_SEEK;
            cur = (char) fgetc(file);
            if (cur < c) {
                index = j;
                c = cur;
            }
        }
        if (index != i) {
            my_log("\tSwaping blocks %lu <-> %lu...\n", i, index);
            if (fseek(file, i * sizeOfBlock, SEEK_SET) != 0)return FILE_SEEK;
            if (fread(block1, sizeof(char), sizeOfBlock, file) != sizeOfBlock) return FILE_READING;

            if (fseek(file, index * sizeOfBlock, SEEK_SET) != 0)return FILE_SEEK;
            if (fread(block2, sizeof(char), sizeOfBlock, file) != sizeOfBlock) return FILE_READING;

            if (fseek(file, index * sizeOfBlock, SEEK_SET) != 0)return FILE_SEEK;
            if (fwrite(block1, sizeof(char), sizeOfBlock, file) != sizeOfBlock) return FILE_WRITING;

            if (fseek(file, i * sizeOfBlock, SEEK_SET) != 0)return FILE_SEEK;
            if (fwrite(block2, sizeof(char), sizeOfBlock, file) != sizeOfBlock) return FILE_WRITING;
        }
    }

    my_log("\tClosing file...\n");
    fclose(file);

    my_log("\tFreeing memory\n");
    free(block1);
    free(block2);

    return NO_ERROR;
}

enum ERRORS sort_sys(char *fileName, size_t numberOfRecords, size_t sizeOfBlock) {
    my_headers("SORT SYS:\t");
    my_log("\n");

    my_log("\tOpening file %s...\n", fileName);
    int file = open(fileName, O_RDWR | O_CREAT);
    if (errno != 0)return FILE_OPENING;

    my_log("\tChecking file size...\n");
    int size = (int) lseek(file, 0, SEEK_END);
    if (errno != 0)return FILE_SEEK;
    if (size < numberOfRecords * sizeOfBlock) return FILE_SIZE;
    lseek(file, 0, SEEK_SET);
    if (errno != 0)return FILE_SEEK;

    my_log("\tCreating blocks...\n");
    char *block1 = malloc(sizeof(char) * (sizeOfBlock + 1));
    char *block2 = malloc(sizeof(char) * (sizeOfBlock + 1));
    char c;
    char cur;
    size_t index;
    size_t j;

    my_log("\tSelection sort...\n");
    size_t i = 0;
    for (; i < numberOfRecords; i++) {
        index = i;
        j = i;

        if (lseek(file, i * sizeOfBlock, SEEK_SET) == -1)return FILE_SEEK;
        if (read(file, &c, sizeof(char)) == -1)return FILE_READING;
        for (; j < numberOfRecords; j++) {
            if (lseek(file, j * sizeOfBlock, SEEK_SET) == -1)return FILE_SEEK;
            if (read(file, &cur, sizeof(char)) == -1)return FILE_READING;
            if (cur < c) {
                index = j;
                c = cur;
            }
        }
        if (index != i) {
            my_log("\tSwaping blocks %lu <-> %lu...\n", i, index);
            if (lseek(file, i * sizeOfBlock, SEEK_SET) == -1)return FILE_SEEK;
            if (read(file, block1, sizeof(char) * sizeOfBlock) == -1) return FILE_READING;

            if (lseek(file, index * sizeOfBlock, SEEK_SET) == -1)return FILE_SEEK;
            if (read(file, block2, sizeof(char) * sizeOfBlock) == -1) return FILE_READING;

            if (lseek(file, index * sizeOfBlock, SEEK_SET) == -1)return FILE_SEEK;
            if (write(file, block1, sizeof(char) * sizeOfBlock) == -1) return FILE_WRITING;

            if (lseek(file, i * sizeOfBlock, SEEK_SET) == -1)return FILE_SEEK;
            if (write(file, block2, sizeof(char) * sizeOfBlock) == -1) return FILE_WRITING;
        }
    }

    my_log("\tClosing file...\n");
    close(file);

    my_log("\tFreeing memory\n");
    free(block1);
    free(block2);

    return NO_ERROR;
}

enum ERRORS copy_lib(char *sourceName, char *destinationName, size_t numberOfRecords, size_t sizeOfBlock) {
    my_headers("COPY LIB:\t");
    my_log("\n");

    my_log("\tOpening source file %s...\n", sourceName);
    FILE *src = fopen(sourceName, "r");
    if (src == NULL) {
        return FILE_OPENING;
    }
    if (fseek(src, 0, SEEK_END) != 0)return FILE_SEEK;
    if (ftell(src) < numberOfRecords * sizeOfBlock) {
        return FILE_SIZE;
    }
    if (fseek(src, 0, SEEK_SET) != 0)return FILE_SEEK;

    my_log("\tOpening destination file %s...\n", destinationName);
    FILE *dest = fopen(destinationName, "w");
    if (dest == NULL) {
        return FILE_OPENING;
    }

    my_log("\tCreating block...\n");
    char *block = malloc(sizeof(char) * (sizeOfBlock + 1));

    my_log("\tCopying block by block...\n");
    size_t i = 0;
    for (; i < numberOfRecords; i++) {
        if (fseek(src, i * sizeOfBlock, SEEK_SET) != 0)return FILE_SEEK;
        fread(block, sizeof(char), sizeOfBlock, src);
        fwrite(block, sizeof(char), sizeOfBlock, dest);
    }

    my_log("\tClosing files...\n");
    fclose(src);
    fclose(dest);

    my_log("\tFreeing memory...\n");
    free(block);
    return NO_ERROR;
}

enum ERRORS copy_sys(char *sourceName, char *destinationName, size_t numberOfRecords, size_t sizeOfBlock) {
    my_headers("COPY SYS:\t");
    my_log("\n");

    my_log("\tOpening source file %s...\n", sourceName);

    int src = open(sourceName, O_RDONLY);
    if (errno != 0)return FILE_OPENING;

    int size;
    if ((size = (int)lseek(src, 0, SEEK_END)) == -1)return FILE_SEEK;
    if (size < numberOfRecords * sizeOfBlock) return FILE_SIZE;
    if (lseek(src, 0, SEEK_SET) == -1)return FILE_SEEK;

    my_log("\tOpening destination file %s...\n", destinationName);
    int dest = open(destinationName, O_WRONLY | O_TRUNC);
    if (errno != 0)return FILE_OPENING;

    my_log("\tCreating block...\n");
    char *block = malloc(sizeof(char) * (sizeOfBlock + 1));

    my_log("\tCopying block by block...\n");
    size_t i = 0;
    for (; i < numberOfRecords; i++) {
        if (lseek(src, i * sizeOfBlock, SEEK_SET) == -1)return FILE_SEEK;
        read(src, block, sizeof(char) * sizeOfBlock);
        write(dest, block, sizeof(char) * sizeOfBlock);
    }

    my_log("\tClosing files...\n");
    close(src);
    close(dest);

    my_log("\tFreeing memory...\n");
    free(block);
    return NO_ERROR;
}

void displayError(char *prefix, enum ERRORS error) {
    switch (error) {
        case FILE_OPENING:
            printf("Error in %s: file opening\n", prefix);
            break;
        case MEMORY_ALLOCATION:
            printf("Error in %s: memory allocation\n", prefix);
            break;
        case FILE_WRITING:
            printf("Error in %s: file writing\n", prefix);
            break;
        case UNKNOWN_TYPE:
            printf("Error in %s: unknown type\n", prefix);
            break;
        case NO_ERROR:
            break;
        case FILE_READING:
            printf("Error in %s: file reading\n", prefix);
            break;
        case FILE_SIZE:
            printf("Error in %s: file size\n", prefix);
            break;
        case FILE_SEEK:
            printf("Error in %s: file seek\n", prefix);
            break;
    }
}

int generate_preprepare(int argc, char **argv) {
    if (argc < 5) {
        printf("Generate expects at 3 arguments: [file name] [number of records] [size of block]\n");
        return 1;
    }
    size_t numberOfRecords = (size_t) strtol(argv[3], NULL, 10);
    size_t sizeOfBlock = (size_t) strtol(argv[4], NULL, 10);
    if (errno != 0) {
        printf("Error while converting arguments\n");
        return 1;
    }
    enum ERRORS ret = generate(argv[2], numberOfRecords, sizeOfBlock);
    displayError("generate", ret);
    if (ret == NO_ERROR)return 0;
    else return 1;
}

int sort_preprepare(int argc, char **argv) {
    if (argc < 6) {
        printf("Sort expects 4 arguments: [file name] [number of records] [size of block] [type]\n");
        return 1;
    }
    enum TYPE type = NO_TYPE;
    if (strcmp(argv[5], "sys") == 0)type = SYS;
    else if (strcmp(argv[5], "lib") == 0)type = LIB;
    size_t numberOfRecords = (size_t) strtol(argv[3], NULL, 10);
    size_t sizeOfBlock = (size_t) strtol(argv[4], NULL, 10);
    if (errno != 0) {
        printf("Error while converting arguments\n");
        return 1;
    }
    enum ERRORS ret;
    switch (type) {
        case LIB:
            ret = sort_lib(argv[2], numberOfRecords, sizeOfBlock);
            break;
        case SYS:
            ret = sort_sys(argv[2], numberOfRecords, sizeOfBlock);
            break;
        default:
            ret = UNKNOWN_TYPE;
            break;
    }
    displayError("sort", ret);
    if (ret == NO_ERROR)return 0;
    else return 1;
}

int copy_preprepare(int argc, char **argv) {
    if (argc < 7) {
        printf("Copy expects 5 arguments: [source file name] [destination file name] [number of records] [size of block] [type]\n");
        return 1;
    }
    enum TYPE type = NO_TYPE;
    if (strcmp(argv[6], "sys") == 0)type = SYS;
    else if (strcmp(argv[6], "lib") == 0)type = LIB;
    size_t numberOfRecords = (size_t) strtol(argv[4], NULL, 10);
    size_t sizeOfBlock = (size_t) strtol(argv[5], NULL, 10);
    if (errno != 0) {
        printf("Error while converting arguments\n");
        return 1;
    }
    enum ERRORS ret;
    switch (type) {
        case SYS:
            ret = copy_sys(argv[2], argv[3], numberOfRecords, sizeOfBlock);
            break;
        case LIB:
            ret = copy_lib(argv[2], argv[3], numberOfRecords, sizeOfBlock);
            break;
        default:
            ret = UNKNOWN_TYPE;
            break;
    }
    displayError("copy", ret);
    if (ret == NO_ERROR)return 0;
    else return 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Program expects at least 1 arguments: [command]\n");
        return 1;
    }

    int ret = 0;

    struct timespec start, stop;
    double dur;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        printf("Problem with clock\n");
        return 1;
    }

    if (strcmp(argv[1], "generate") == 0)ret = generate_preprepare(argc, argv);
    else if (strcmp(argv[1], "sort") == 0)ret = sort_preprepare(argc, argv);
    else if (strcmp(argv[1], "copy") == 0)ret = copy_preprepare(argc, argv);
    else {
        printf("Command %s not known\n", argv[1]);
        ret = 1;
    }

    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        printf("Problem with clock\n");
        return 1;
    }
    dur = stop.tv_sec - start.tv_sec + (stop.tv_nsec - start.tv_nsec) * 1.0 / 1000000000;

    printf("%lf s\n", dur);
    return ret;
}