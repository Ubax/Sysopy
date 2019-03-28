#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

void toUpper(char *str) {
    size_t i = 0;
    for (; i < strlen(str); i++)str[i] = (char) toupper(str[i]);
}

int compareArg(char ** argv, int id, const char * value){
    char arg[strlen(argv[id])+1];
    char val[strlen(value)+1];
    strcpy(arg, argv[id]);
    strcpy(val, value);
    toUpper(arg);
    toUpper(val);
    return strcmp(arg, val)==0;
}

int getArgAsInt(char ** argv, int id){
    return (int) strtol(argv[id], NULL, 10);
}

size_t getArgAsSizeT(char ** argv, int id){
    int ret = (int) strtol(argv[id], NULL, 10);
    if(ret < 1){
        printf("Number of signals should be positive\n");
        return 1;
    }
    return (size_t)ret;
}