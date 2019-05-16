#include <pthread.h>

#ifndef IMAGE_H
#define IMAGE_H

enum MODE { BLOCK, INTERLEAVED };

struct IMAGE {
  int width, height, maxColor;
  int **data;
};

struct FILTER {
  int size;
  float **data;
};

struct FILTER_ARGS {
  int numberOfThreads;
  int threadId;
  struct IMAGE *inputImage;
  struct IMAGE *outputImage;
  struct FILTER *filter;
};

struct IMAGE createEmptyImage(int width, int height);
struct IMAGE loadImage(char *fileName);
void saveImage(char *fileName, struct IMAGE *image);
void clearImage(struct IMAGE *image);

struct FILTER loadFilter(char *fileName);
void clearFilter(struct FILTER *filter);

int ceilDiv(int a, int b);
int _round(float x);
int min(int a, int b);
int max(int a, int b);

#endif
