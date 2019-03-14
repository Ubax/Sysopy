#include "dirReader.h"

enum TYPE getTypeFromString(char *type) {
    if (type[0] == '<')return LATER;
    else if (type[0] == '=')return EXACT;
    else if (type[0] == '>')return EARLIER;
    return NO_TYPE;
}

enum TIME_ERRORS{
    TIME_NO_ERROR=-1,
    YEAR=0,
    MONTH=1,
    DAY=2,
    HOUR=3,
    MINUTE=4,
    SECOND=5,
    DECIMAL=6
};


const char * TIME_ERRORS_STRING[] = {
        "YEAR",
        "MONTH",
        "DAY",
        "HOUR",
        "MINUTE",
        "SECOND",
        "DECIMAL"
};

enum TIME_ERRORS stringToTM(char *date, struct tm *dateTm) {
    /**
     * TIME FORMAT
     * YYYY-MM-DD,HH:MM:SS
     */
    size_t i = 0;
    int buf = 0;
    for (; i < strlen(date); i++) {
        if (date[i] == '-')break;
        if (date[i] < '0' || date[i] > '9')return DECIMAL;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 1900 || buf > 2100) return YEAR;

    dateTm->tm_year = buf - 1900;
    buf = 0; i++;
    for (; i < strlen(date); i++) {
        if (date[i] == '-')break;
        if (date[i] < '0' || date[i] > '9')return DECIMAL;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 1 || buf > 12) return MONTH;
    dateTm->tm_mon = buf - 1;
    buf = 0; i++;

    for (; i < strlen(date); i++) {
        if (date[i] == ',')break;
        if (date[i] < '0' || date[i] > '9')return DECIMAL;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 1 || buf > 31) return DAY;
    dateTm->tm_mday = buf;
    buf = 0; i++;

    for (; i < strlen(date); i++) {
        if (date[i] == ':')break;
        if (date[i] < '0' || date[i] > '9')return DECIMAL;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 0 || buf > 23) return HOUR;
    dateTm->tm_hour = buf;
    buf = 0; i++;

    for (; i < strlen(date); i++) {
        if (date[i] == ':')break;
        if (date[i] < '0' || date[i] > '9')return DECIMAL;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 0 || buf > 59) return MINUTE;
    dateTm->tm_min = buf;
    buf = 0; i++;

    for (; i < strlen(date); i++) {
        if (date[i] == ':')break;
        if (date[i] < '0' || date[i] > '9')return DECIMAL;
        buf = buf * 10 + date[i] - '0';
    }
    if (buf < 0 || buf > 59) return SECOND;
    dateTm->tm_sec = buf;
    return TIME_NO_ERROR;
}

int ors_preprepare(char *dir, char *type, char *date) { // opendir, readdir, stat
    struct tm dateTm;
    enum TIME_ERRORS err;
    if((err=stringToTM(date, &dateTm))!=TIME_NO_ERROR){
        printf("Wrong date format: %s\n", TIME_ERRORS_STRING[err]);
        return 1;
    }

    printf("File Name\t\t\tModification Date\tAccess Date\t\tType\tSize\n");
    enum ERRORS error = ors(realpath(dir, NULL), getTypeFromString(type), dateTm);
    displayError("ORS", error);
    if (error != NO_ERROR)return 1;
    return 0;
}

int n_preprepaer(char *dir, char *type, char *date) { //nftw
    struct tm dateTm;
    enum TIME_ERRORS err;
    if((err=stringToTM(date, &dateTm))!=TIME_NO_ERROR){
        printf("Wrong date format: %s\n", TIME_ERRORS_STRING[err]);
        return 1;
    }

    enum ERRORS error = n(realpath(dir, NULL), getTypeFromString(type), dateTm);
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

    printf("\n\n\n");

    if (n_preprepaer(argv[1], argv[2], argv[3]) != 0)return 1;
    return 0;
}

