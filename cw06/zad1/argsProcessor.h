#ifndef SYSOPY_ARGSPROCESSOR_H
#define SYSOPY_ARGSPROCESSOR_H

int compareArg(char ** argv, int id, const char * value);
int compare(char * template, const char * value);
int getArgAsInt(char ** argv, int id);
size_t getArgAsSizeT(char ** argv, int id);
void toUpper(char *str);

#endif //SYSOPY_ARGSPROCESSOR_H
