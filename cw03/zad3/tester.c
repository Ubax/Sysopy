#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define DATE_FORMAT "%F_%H-%M-%S"

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("Program expects at 4 argument: [file name] [p min] [p max] [bytes]\n");
        return 1;
    }
    char *fileName = argv[1];
    int pMin = strtol(argv[2], NULL, 10);
    int pMax = strtol(argv[3], NULL, 10);
    int bytes = strtol(argv[4], NULL, 10);
    printf("Bytes: %i\n", bytes);
    if (pMin <= 0 || pMax <= 0 || pMin >= pMax) {
        printf("p min and max should be positive and pmin<pmax\n");
        return 1;
    }
    if (bytes <= 0) {
        printf("Bytes should be positive\n");
        return 1;
    }
    char *line;
    while (1) {
        unsigned secs = rand() % (pMin - pMax + 1) + pMin;
        sleep(secs);

        char pid_str[12];
        sprintf(pid_str, "%d", getpid());
        char sec_str[12];
        sprintf(sec_str, "%d", secs);
        char time_str[30];
        time_t currentTime;
        time(&currentTime);
        strftime(time_str, 29, DATE_FORMAT, localtime(&currentTime));

        line = malloc(sizeof(char)*(bytes + 200));
        strcpy(line, pid_str);
        strcat(line, " ");
        strcat(line, sec_str);
        strcat(line, " ");
        strcat(line, time_str);
        char bytes_str[bytes+1];
        size_t i=0;
        for(;i<bytes;i++)bytes_str[i]=(char)rand()%50+65;
        bytes_str[bytes]='\0';
        strcat(line, bytes_str);
        strcat(line, "\n");

        FILE *file = fopen(fileName, "a");
        if (file == NULL) {
            printf("Problem with opening file: %s\n", fileName);
        }

        fwrite(line, sizeof(char), strlen(line), file);

        fclose(file);

        free(line);
        printf("Data written to: %s\n", fileName);

    }

    return 0;
}
