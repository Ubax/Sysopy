#include "image.h"
#include "sysopy.h"
#include <stdio.h>
#include <stdlib.h>

int ceilDiv(int a, int b) { return a / b + (a % b != 0); }

int _round(float x) {
  if (x < (int)x + 0.5)
    return (int)x;
  return (int)x + 1;
}

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

struct IMAGE createEmptyImage(int width, int height) {
  struct IMAGE retImage;
  int i, j;
  retImage.width = width;
  retImage.height = height;
  retImage.maxColor = 255;
  retImage.data = malloc(width * sizeof(int *));
  if (retImage.data == NULL) {
    ERROR_EXIT("Allocating memory for image");
  }

  for (i = 0; i < width; i++) {
    retImage.data[i] = malloc(height * sizeof(int));
    if (retImage.data[i] == NULL) {
      ERROR_EXIT("Allocating memory for image");
    }
    for (j = 0; j < height; j++) {
      retImage.data[i][j] = 0;
    }
  }

  return retImage;
}

struct IMAGE loadImage(char *fileName) {
  struct IMAGE retImage;
  int width = 0, height = 0, maxColor;
  int i, j;
  char bufor[255];
  FILE *file;
  if ((file = fopen(fileName, "r")) == NULL)
    ERROR_EXIT("Load image - file opening");
  fscanf(file, "%s", bufor);
  if (strcmp(bufor, "P2") != 0)
    MESSAGE_EXIT("Wrong file header");
  fscanf(file, "%i %i %i", &width, &height, &maxColor);
  if (width < 0 || height < 0) {
    ERROR_EXIT("Size loading");
  }
  retImage = createEmptyImage(width, height);
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      fscanf(file, "%i", &retImage.data[j][i]);
    }
  }
  fclose(file);
  return retImage;
}

void saveImage(char *fileName, struct IMAGE *image) {
  FILE *file;
  int i, j;
  int charsInLine = 0;
  if ((file = fopen(fileName, "w")) == NULL)
    ERROR_EXIT("Save image - file opening");

  fprintf(file, "P2\n%i %i\n%i\n", image->width, image->height,
          image->maxColor);

  for (i = 0; i < image->height; i++) {
    for (j = 0; j < image->width; j++) {
      fprintf(file, "%i ", image->data[j][i]);
      charsInLine++;
      if (charsInLine >= 70) {
        charsInLine = 0;
        fprintf(file, "\n");
      }
    }
  }
  fclose(file);
}

void clearImage(struct IMAGE *image) {
  int i;
  for (i = 0; i < image->width; i++) {
    free(image->data[i]);
  }
  free(image->data);
}

struct FILTER createEmptyFilter(int size) {
  struct FILTER retFilter;
  int i, j;
  retFilter.size = size;
  retFilter.data = malloc(size * sizeof(float *));
  if (retFilter.data == NULL) {
    ERROR_EXIT("Allocating memory for image");
  }

  for (i = 0; i < size; i++) {
    retFilter.data[i] = malloc(size * sizeof(float));
    if (retFilter.data[i] == NULL) {
      ERROR_EXIT("Allocating memory for image");
    }
    for (j = 0; j < size; j++) {
      retFilter.data[i][j] = 0.0;
    }
  }

  return retFilter;
}

struct FILTER loadFilter(char *fileName) {
  struct FILTER retFilter;
  int size;
  int i, j;
  FILE *file;
  if ((file = fopen(fileName, "r")) == NULL)
    ERROR_EXIT("Load filter - file opening");
  fscanf(file, "%i", &size);
  retFilter = createEmptyFilter(size);
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      fscanf(file, "%f", &retFilter.data[j][i]);
    }
  }
  fclose(file);
  return retFilter;
}

void clearFilter(struct FILTER *filter) {
  int i;
  for (i = 0; i < filter->size; i++) {
    free(filter->data[i]);
  }
  free(filter->data);
}
