#include "image.h"
#include "sysopy.h"

void createThreads();

int numberOfThreads = 0;
enum MODE mode = BLOCK;
char *inputFileName, *outputFileName, *filterFileName;

pthread_t *threads = NULL;

struct IMAGE inputImage, outputImage;
struct FILTER_ARGS *filterArgs;

int main(int argc, char **argv) {
  if (argc < 6) {
    MESSAGE_EXIT("Program expects 5 arguments: "
                 "number_of_threads\t"
                 "mode [block/interleaved]\t"
                 "input file name\t"
                 "filter file name\t"
                 "output file name\t");
  }
  numberOfThreads = getArgAsInt(argv, 1);
  if (compareArg(argv, 2, "INTERLEAVED"))
    mode = INTERLEAVED;
  inputFileName = argv[3];
  filterFileName = argv[4];
  outputFileName = argv[5];

  threads = malloc(sizeof(pthread_t) * numberOfThreads);
  if (threads == NULL)
    ERROR_EXIT("Allocating threads");
  filterArgs = malloc(sizeof(struct FILTER_ARGS) * numberOfThreads);
  if (filterArgs == NULL)
    ERROR_EXIT("Allocating args");

  inputImage = loadImage(inputFileName);
  outputImage = createEmptyImage(inputImage.width, inputImage.height);

  createThreads();

  int i;
  for (i = 0; i < numberOfThreads; i++) {
    pthread_join(threads[i], NULL);
  }

  saveImage(outputFileName, &outputImage);

  return 0;
}

void cleanExit() {
  if (threads != NULL)
    free(threads);
  if (filterArgs != NULL)
    free(filterArgs);
  clearImage(&inputImage);
  if (outputImage.data != NULL)
    clearImage(&outputImage);
}

void createThreads() {
  int i;
  for (i = 0; i < numberOfThreads; i++) {
    filterArgs[i].numberOfThreads = numberOfThreads;
    filterArgs[i].inputImage = &inputImage;
    filterArgs[i].outputImage = &outputImage;
    filterArgs[i].threadId = i;
    pthread_create(&threads[i], NULL, blockFilter, &filterArgs[i]);
  }
}

void transformColumn(struct FILTER_ARGS *args, int columnId) {
  int y, i, j;
  int c = args->filter->size;
  double s;
  for (y = 0; y < args->inputImage->height; y++) {
    s = 0.0;
    for (i = 0; i < c; i++) {
      for (j = 0; j < c; j++) {
        s += args->inputImage->data[max(1, columnId - ceilDiv(c, 2) + i)]
                                   [max(1, y - ceilDiv(c, 2) + j)] *
             args->filter->data[i][j];
      }
    }
    args->outputImage->data[columnId][y] = _round(s);
  }
}

void *blockFilter(void *args) {
  struct FILTER_ARGS *filterArgs = args;
  int i;
  int min = filterArgs->threadId * ceilDiv(filterArgs->outputImage->width,
                                           filterArgs->numberOfThreads);
  int max = (filterArgs->threadId + 1) * ceilDiv(filterArgs->outputImage->width,
                                                 filterArgs->numberOfThreads);

  for (i = min; i < max; i++) {
    transformColumn(filterArgs, i);
  }
  return (void *)0;
}
