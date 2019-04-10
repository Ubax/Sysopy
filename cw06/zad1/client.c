#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "argsProcessor.h"

enum INPUT_TYPE {
    FROM_FILE,
    FROM_TERMINAL
};

enum INPUT_TYPE input_type = FROM_TERMINAL;
char fileName[FILENAME_MAX];

char **tokenizer(char *str, int numberOfTokens) {
    if (numberOfTokens <= 0) {
        printf("Not positive number of tokens\n");
        return NULL;
    }
    size_t len = strlen(str);

    char **tokensArray = malloc(numberOfTokens * sizeof(char *));
    if(tokensArray==NULL){
        printf("Error while allocating memory\n");
        exit(1);
    }
    if (numberOfTokens == 1) {
        tokensArray[0] = str;
        return tokensArray;
    }
    size_t tokenNum = 0;
    if((tokensArray[0] = malloc((len + 1) * sizeof(char)))==NULL){
        printf("Error while allocating memory\n");
        exit(1);
    }
    size_t posInToken=0;
    size_t i = 0;
    for (; i < len; i++) {
        if (str[i] == ' ' || str[i] == '\t') {
            tokensArray[tokenNum][posInToken]='\0';
            if((tokensArray[++tokenNum] = malloc((len - i + 1) * sizeof(char)))==NULL){
                printf("Error while allocating memory\n");
                exit(1);
            }
            posInToken=0;
            if(tokenNum == numberOfTokens-1)break;
        }else{
            tokensArray[tokenNum][posInToken]=str[i];
            posInToken++;
        }
    }
    if(tokenNum<numberOfTokens-1){
        printf("To few tokens");
        i = 0;
        for (; i < tokenNum; i++) {
            free(tokensArray[i]);
        }
        return NULL;
    }
    for (; i < len; i++) {
        tokensArray[tokenNum][posInToken]=str[i];
        posInToken++;
    }
    return tokensArray;
}

int runCommand(char *command) {
    char **tokensCommand;
    tokensCommand = tokenizer(command, 2);
    if (compare(tokensCommand[0], "ECHO")) {

        printf("echo\n");
    } else if (compare(tokensCommand[0], "LIST")) {
        printf("list\n");
    } else if (compare(tokensCommand[0], "FRIENDS")) {
        printf("friends\n");
    } else if (compare(tokensCommand[0], "2ALL")) {
        printf("2all\n");
    } else if (compare(tokensCommand[0], "2FRIENDS")) {
        printf("2friends\n");
    } else if (compare(tokensCommand[0], "2ONE")) {
        printf("2one\n");
    } else if (compare(tokensCommand[0], "STOP")) {
        printf("stop\n");
    }
    free(tokensCommand[0]);
    free(tokensCommand[1]);
    free(tokensCommand);
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 2) {
        if (compareArg(argv, 1, "FILE")) {
            if (argc < 3) {
                printf("Type file expects one more argument: [file name]\n");
                strcpy(fileName, argv[2]);
                return 1;
            }
            input_type = FROM_FILE;
        }
        return 1;
    }

    return 0;
}

