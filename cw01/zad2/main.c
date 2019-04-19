#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdarg.h>
#include "library.h"


#define CONSOLE 0
#define NUMBER_OF_COMMANDS 6

int searchDirectory(struct Array *array, char **args) {
    my_log("\tsearch_directory...\n");
    char *dir;
    char *file;
    char *fileTmp;

    dir = args[0];
    file = args[1];
    fileTmp = args[2];

    if (dir == NULL || strlen(dir) == 0 || file == NULL || strlen(file) == 0 || fileTmp == NULL ||
        strlen(fileTmp) == 0)
        return 3;
    my_log("\tall arguments seem to be correct...\n");

    findAndSaveResultToTemporaryFile(dir, file, fileTmp);
    return -1;
}

int addToTable(struct Array *array, char **args) {
    my_log("\tadd_to_table...\n");
    char *fileTmp = args[0];
    if (fileTmp == NULL || strlen(fileTmp) == 0)
        return 2;
    my_log("\tall arguments seem to be correct...\n");
    addTemporaryFileBlockPointerToArray(array, fileTmp);
    return -1;
}

int main(int argc, char **argv) {
    my_log("Systemy Operacyjne 2019\nAuthor: Jakub Tkacz\nVesrion:1.1\nDate:10.03.2019\n\n");
    struct Array array = {NULL, 0};

    struct timespec start, stop;
    double dur;
    static struct tms st_cpu;
    static struct tms en_cpu;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        printf("Problem with clock\n");
        return 1;
    }


    times(&st_cpu);

    int exitCode = argumentList(argc, argv, &array);

    times(&en_cpu);

    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        printf("Problem with clock\n");
        return 1;
    }

    dur = stop.tv_sec - start.tv_sec + (stop.tv_nsec - start.tv_nsec)*1.0/1000000000;

    printf("Real time: %lf s\n", dur);
    printf("Cpu user time: %Lf s\n", (long double)(en_cpu.tms_utime - st_cpu.tms_utime)/sysconf(_SC_CLK_TCK));
    printf("Cpu system time: %Lf s\n", (long double)(en_cpu.tms_stime - st_cpu.tms_stime)/sysconf(_SC_CLK_TCK));

    FILE* fp = fopen("raport2.txt", "a");
    fprintf(fp, "Real time: %lf s\nCpu user time: %Lf s\nCpu system time: %Lf s\n",dur,(long double)(en_cpu.tms_utime - st_cpu.tms_utime)/sysconf(_SC_CLK_TCK),(long double)(en_cpu.tms_stime - st_cpu.tms_stime)/sysconf(_SC_CLK_TCK));
    fclose(fp);
#if CONSOLE > 0
    while (exitCode == -1) {
        printf("\n> ");
        exitCode = console(&array);
    }
#endif
    emptyArrayAndBlocks(&array);
    return exitCode == -1 ? 0 : exitCode;
}
