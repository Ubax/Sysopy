#include "dirReader.h"

enum TYPE getTypeFromString(char *type) {
    if (type[0] == '<')return LATER;
    else if (type[0] == '=')return EXACT;
    else if (type[0] == '>')return EARLIER;
    return NO_TYPE;
}

int stringToTM(char *date, struct tm *dateTm) {
    /**
     * TIME FORMAT
     * YYYY-MM-DD,HH:MM:SS
     */
    size_t i = 0;
    int buf = 0;
    for (; i < strlen(date); i++) {
        if (date[i] == '-')break;
        if (date[i] < '0' || date[i] > '9')return 1;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 1900 || buf > 2100) return 1;

    dateTm->tm_year = buf - 1900;
    buf = 0; i++;
    for (; i < strlen(date); i++) {
        if (date[i] == '-')break;
        if (date[i] < '0' || date[i] > '9')return 1;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 1 || buf > 12) return 1;
    dateTm->tm_mon = buf - 1;
    buf = 0; i++;

    for (; i < strlen(date); i++) {
        if (date[i] == ',')break;
        if (date[i] < '0' || date[i] > '9')return 1;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 1 || buf > 31) return 1;
    dateTm->tm_mday = buf;
    buf = 0; i++;

    for (; i < strlen(date); i++) {
        if (date[i] == ':')break;
        if (date[i] < '0' || date[i] > '9')return 1;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 0 || buf > 23) return 1;
    dateTm->tm_hour = buf;
    buf = 0; i++;

    for (; i < strlen(date); i++) {
        if (date[i] == ':')break;
        if (date[i] < '0' || date[i] > '9')return 1;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 0 || buf > 59) return 1;
    dateTm->tm_min = buf;
    buf = 0; i++;

    for (; i < strlen(date); i++) {
        if (date[i] == ':')break;
        if (date[i] < '0' || date[i] > '9')return 1;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 0 || buf > 59) return 1;
    dateTm->tm_sec = buf;
    buf = 0; i++;
    return 0;
}

int ors_preprepare(char *dir, char *type, char *date) { // opendir, readdir, stat
    struct tm dateTm;
    if(stringToTM(date, &dateTm)!=0){
        printf("Wrong date format\n");
        return 1;
    }
    enum ERRORS error = ors(dir, getTypeFromString(type), dateTm);
    displayError("ORS", error);
    if (error != NO_ERROR)return 1;
    return 0;
}

int n_preprepaer(char *dir, char *type, char *date) { //nftw
    struct tm dateTm;
    if(stringToTM(date, &dateTm)!=0){
        printf("Wrong date format\n");
        return 1;
    }
    enum ERRORS error = n(dir, getTypeFromString(type), dateTm);
    displayError("NFTW", error);
    if (error != NO_ERROR)return 1;
    return 0;
};

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Program expects at 3 arguments: [directory] [type (<,=,>)] [date]\n");
        return 1;
    }
    if (strcmp(argv[2], "<") != 0 && strcmp(argv[2], "=") != 0 && strcmp(argv[2], ">") != 0) {
        printf("Second argument should be one of < = >\n");
        return 1;
    }

    if (ors_preprepare(argv[1], argv[2], argv[3]) != 0)return 1;

    if (n_preprepaer(argv[1], argv[2], argv[3]) != 0)return 1;
    return 0;
}

