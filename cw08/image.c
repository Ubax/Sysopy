#include "image.h"
#include "sysopy.h"
#include <stdio.h>
#include <stdlib.h>

int ceilDiv(int a, int b) { return a / b + (a % b != 0); }

int _round(double x) {
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
    retImage.data[i] = malloc(width * sizeof(int));
    for (j = 0; j < height; j++) {
      retImage.data[i][j] = 0;
    }
  }

  return retImage;
}

struct IMAGE loadImage(char *fileName) {
  struct IMAGE retImage;
  int width, height, maxColor;
  int i, j;
  char bufor[255];
  FILE *file;
  if ((file = fopen(fileName, "r")) == NULL)
    ERROR_EXIT("Load image - file opening");
  fscanf(file, "%s", bufor);
  if (strcmp(bufor, "P2") != 0)
    MESSAGE_EXIT("Wrong file header");
  fscanf(file, "%i %i %i", &width, &height, &maxColor);
  retImage = createEmptyImage(width, height);
  for (i = 0; i < width; i++) {
    for (j = 0; j < height; j++) {
      fscanf(file, "%i", &retImage.data[i][j]);
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
