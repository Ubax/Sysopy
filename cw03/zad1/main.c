#include "dirReader.h"


int ors_preprepare(char *dir) { // opendir, readdir, stat

    enum ERRORS error = ors(dir);
    displayError("ORS", error);
    if (error != NO_ERROR)return 1;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Program expects at 1 argument: [directory]\n");
        return 1;
    }

    if (ors_preprepare(argv[1]) != 0)return 1;
    return 0;
}

