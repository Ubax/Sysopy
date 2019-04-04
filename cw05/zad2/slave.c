#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int getArgAsInt(char ** argv, int id){
    return (int) strtol(argv[id], NULL, 10);
}

#define LINE_MAX_LENGTH 128
#define DATE_MAX_LENGTH 64

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Program expects at last 2 argument: [fifo name] [N - number of messages]\n");
        return 1;
    }
    FILE* file = fopen(argv[1], "w");
    if(file==NULL){
        perror("FIFO OPENING");
        return 1;
    }
    int i=0;
    char date[DATE_MAX_LENGTH];
    char line[LINE_MAX_LENGTH];
    for(;i<getArgAsInt(argv, 2);i++){
        FILE* dateWriter = popen("date", "r");
        fread(date, 1, DATE_MAX_LENGTH, dateWriter);
        fclose(dateWriter);
        sleep(rand()%4+2);
        sprintf(line, "%i\t%s", getpid(), date);
        fwrite(line, 1, LINE_MAX_LENGTH, file);
    }
    fclose(file);
    return 0;
}