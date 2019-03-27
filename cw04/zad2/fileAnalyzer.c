size_t getNumberOfLines(char *fileName) {
    FILE *file = fopen(fileName, "r");
    char *line = NULL;
    size_t size = 0;
    size_t numberOfLines = 0;
    while (getline(&line, &size, file) != -1) {
        numberOfLines++;
        free(line);
    }
    return numberOfLines;
}

struct FILE_RECORD getFileRecord(char *line) {
    size_t i = 0;
    int deli = 0;
    for (; i < strlen(line); i++) {
        if (line[i] == ';') {
            deli = 1;
            break;
        }
    }
    if (deli == 0) {
        printf("No delimeter [;]\n");
        exit(1);
    }
    struct FILE_RECORD fr;
    char *dir = strtok(line, ";");
    char *seconds = strtok(NULL, ";");
    strcpy(fr.dir, dir);
    fr.seconds = (int) strtol(seconds, NULL, 10);
    if (fr.seconds <= 0) {
        printf("getFilesToWatchFromFile: Time should be positive\n");
        exit(1);
    }
    return fr;
}

struct FILES_ARRAY getFilesToWatchFromFile(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("getFilesToWatchFromFile: No such file: %s\n", fileName);
        exit(1);
    }
    struct FILES_ARRAY files_array;
    files_array.size = getNumberOfLines(fileName);
    files_array.files = malloc(sizeof(struct FILE_RECORD) * files_array.size);
    char *line = NULL;
    size_t size = 0;
    int i = 0;
    while (getline(&line, &size, file) != -1) {
        files_array.files[i] = getFileRecord(line);
        i++;
        free(line);
    }
    return files_array;
}