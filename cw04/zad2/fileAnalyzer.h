#include "monitor.h"

#ifndef SYSOPY_FILEANALYZER_H
#define SYSOPY_FILEANALYZER_H

size_t getNumberOfLines(char *fileName);
struct FILE_RECORD getFileRecord(char *line);
struct FILES_ARRAY getFilesToWatchFromFile(char *fileName);

#endif //SYSOPY_FILEANALYZER_H
