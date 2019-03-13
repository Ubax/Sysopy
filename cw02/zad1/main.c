#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

#define LOG 1

static inline void my_log(const char *msg, ...) {
#if LOG > 0
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
};

enum ERRORS generate(char *fileName, size_t numberOfRecords, size_t sizeOfBlock) {
    my_log("Generating...\n");
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
    my_log("Sorting lib...\n");
    my_log("\tOpening file %s...\n", fileName);
    FILE *file = fopen(fileName, "r+");
    if (file == NULL) {
        return FILE_OPENING;
    }
    fseek(file, 0, SEEK_END);
    if (ftell(file) < numberOfRecords * sizeOfBlock) {
        return FILE_SIZE;
    }
    fseek(file, 0, SEEK_SET);
    my_log("\tCreating blocks...\n");
    char *block1 = malloc(sizeof(char) * (sizeOfBlock + 1));
    char *block2 = malloc(sizeof(char) * (sizeOfBlock + 1));
    my_log("\tSelection sort...\n");
    size_t i = 0;
    for (; i < numberOfRecords; i++) {
        size_t index = i;
        size_t j = i;
        fseek(file, i * sizeOfBlock, SEEK_SET);
        char c = (char) fgetc(file);
        char cur;
        for (; j < numberOfRecords; j++) {
            fseek(file, j * sizeOfBlock, SEEK_SET);
            cur = (char) fgetc(file);
            if (cur < c) {
                index = j;
                c = cur;
            }
        }
        if (index != i) {
            my_log("\tSwaping blocks %lu <-> %lu...\n", i, index);
            fseek(file, i * sizeOfBlock, SEEK_SET);
            if (fread(block1, sizeof(char), sizeOfBlock, file) != sizeOfBlock) {
                return FILE_READING;
            }
            fseek(file, index * sizeOfBlock, SEEK_SET);
            if (fread(block2, sizeof(char), sizeOfBlock, file) != sizeOfBlock) {
                return FILE_READING;
            }
            fseek(file, index * sizeOfBlock, SEEK_SET);
            if (fwrite(block1, sizeof(char), sizeOfBlock, file) != sizeOfBlock) {
                return FILE_WRITING;
            }
            fseek(file, i * sizeOfBlock, SEEK_SET);
            if (fwrite(block2, sizeof(char), sizeOfBlock, file) != sizeOfBlock) {
                return FILE_WRITING;
            }
        }
    }
    my_log("\tClosing file...\n");
    fclose(file);
    my_log("\tFreeing memory\t");
    free(block1);
    free(block2);
    return NO_ERROR;
}

enum ERRORS sort_sys(char *fileName, size_t numberOfRecords, size_t sizeOfBlock) {

    return 0;
}

enum ERRORS copy_lib(char *sourceName, char *destinationName, size_t numberOfRecords, size_t sizeOfBlock) {
    my_log("Copying lib...\n");

    my_log("\tOpening source file %s...\n", sourceName);
    FILE *src = fopen(sourceName, "r");
    if (src == NULL) {
        return FILE_OPENING;
    }
    fseek(src, 0, SEEK_END);
    if (ftell(src) < numberOfRecords * sizeOfBlock) {
        return FILE_SIZE;
    }
    fseek(src, 0, SEEK_SET);

    my_log("\tOpening destination file %s...\n", destinationName);
    FILE *dest = fopen(sourceName, "w");
    if (dest == NULL) {
        return FILE_OPENING;
    }

    my_log("\tCreating block...\n");
    char *block = malloc(sizeof(char) * (sizeOfBlock + 1));

    my_log("\tCopying block by block...\n");
    size_t i = 0;
    for (; i < numberOfRecords; i++) {
        fseek(src, i * sizeOfBlock, SEEK_SET);
        fread(block, sizeof(char), sizeOfBlock, src);
        fwrite(dest, sizeof(char), sizeOfBlock, dest);
    }

    my_log("\tClosing files...\n");
    fclose(src);
    fclose(dest);

    my_log("\tFreeing memory...\n");
    free(block);
    return 0;
}

enum ERRORS copy_sys(char *sourceName, char *destinationName, size_t numberOfRecords, size_t sizeOfBlock) {
    return 0;
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
    }
}

int generate_preprepare(char *fileName, size_t numberOfRecords, size_t sizeOfBlock) {
    enum ERRORS ret = generate(fileName, numberOfRecords, sizeOfBlock);
    displayError("generate", ret);
    if (ret == NO_ERROR)return 0;
    else return 1;
}

int sort_preprepare(char *fileName, size_t numberOfRecords, size_t sizeOfBlock, enum TYPE type) {
    enum ERRORS ret = UNKNOWN_TYPE;
    switch (type) {
        case LIB:
            ret = sort_lib(fileName, numberOfRecords, sizeOfBlock);
            break;
        case SYS:
            ret = sort_sys(fileName, numberOfRecords, sizeOfBlock);
            break;
        default:
            ret = UNKNOWN_TYPE;
            break;
    }
    displayError("sort", ret);
    if (ret == NO_ERROR)return 0;
    else return 1;
}

int
copy_preprepare(char *sourceName, char *destinationName, size_t numberOfRecords, size_t sizeOfBlock, enum TYPE type) {
    enum ERRORS ret = UNKNOWN_TYPE;
    switch (type) {
        case SYS:
            ret = copy_sys(sourceName, destinationName, numberOfRecords, sizeOfBlock);
            break;
        case LIB:
            ret = copy_lib(sourceName, destinationName, numberOfRecords, sizeOfBlock);
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
    if (argc < 5) {
        printf("Program expects at least 4 arguments: [command] [file name] [number of blocks] [size of block]\n");
        return 1;
    }
    size_t numberOfRecord = (size_t) strtol(argv[3], NULL, 10);
    size_t sizeOfBlock = (size_t) strtol(argv[4], NULL, 10);
    enum TYPE type = NO_TYPE;
    if (errno != 0) {
        printf("Error while converting arguments\n");
        return 1;
    }
    if (strcmp(argv[1], "generate") == 0) {
        return generate_preprepare(argv[2], numberOfRecord, sizeOfBlock);
    } else if (strcmp(argv[1], "sort") == 0) {
        if (argc < 6) {
            printf("Too few arguments for sort\n");
            return 1;
        }
        if (strcmp(argv[5], "sys") == 0)type = SYS;
        else if (strcmp(argv[5], "lib") == 0)type = LIB;
        return sort_preprepare(argv[2], numberOfRecord, sizeOfBlock, type);
    } else if (strcmp(argv[2], "copy") == 0) {
        if (argc < 7) {
            printf("Too few arguments for copy\n");
            return 1;
        }
        if (strcmp(argv[5], "sys") == 0)type = SYS;
        else if (strcmp(argv[5], "lib") == 0)type = LIB;
        return copy_preprepare(argv[2], numberOfRecord, sizeOfBlock, type);
    }
    printf("Command %s not known\n", argv[1]);
    return 0;
}