#include <stdio.h>
#include <string.h>

int generate(char * fileName, size_t numberOfRecords, size_t sizeOfBlock){
    return 0;
}

int sort_lib(char * fileName, size_t numberOfRecords, size_t sizeOfBlock){
    return 0;
}

int sort_sys(char * fileName, size_t numberOfRecords, size_t sizeOfBlock){
    return 0;
}

int copy_lib(char * fileName, size_t numberOfRecords, size_t sizeOfBlock){
    return 0;
}
int copy_sys(char * fileName, size_t numberOfRecords, size_t sizeOfBlock){
    return 0;
}

int main(int argc, char ** argv){
    if(argc < 2){
        printf("No command given\n");
        return 1;
    }
    if(strcmp(argv[1], "generate")==0){
        if(argc < 5){
            printf("Generate expects 3 arguments: [file name] [number of blocks] [size of block]\n");
            return 1;
        }
        return generate(argv[2],1,1);
    }
    else if(strcmp(argv[1], "sort")==0){
        if(argc < 6){
            printf("Generate expects 4 arguments: [file name] [number of blocks] [size of block] [type]\n");
            return 1;
        }
        return sort_sys(argv[2],1,1);
    }
    else if(strcmp(argv[1], "copy")==0){
        if(argc < 6){
            printf("Generate expects 4 arguments: [file name] [number of blocks] [size of block] [type]\n");
            return 1;
        }
        return copy_sys(argv[2],1,1);
    }
    printf("Command %s not known\n", argv[1]);
    return 0;
}