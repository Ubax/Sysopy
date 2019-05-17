#include "image.h"
#include "sysopy.h"

void createThreads();
void *blockFilter(void *);
void *interleavedFilter(void *);

int numberOfThreads = 0;
enum MODE mode = BLOCK;
char *inputFileName, *outputFileName, *filterFileName;

pthread_t *threads = NULL;

struct IMAGE inputImage, outputImage;
struct FILTER filter;
struct FILTER_ARGS *filterArgs;

double **threadsTimes;

int main(int argc, char **argv) {
  int i;
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
  threadsTimes = malloc(sizeof(double *) * numberOfThreads);
  if (threadsTimes == NULL)
    ERROR_EXIT("Allocating threads times");
  filterArgs = malloc(sizeof(struct FILTER_ARGS) * numberOfThreads);
  if (filterArgs == NULL)
    ERROR_EXIT("Allocating args");

  filter = loadFilter(filterFileName);

  inputImage = loadImage(inputFileName);
  outputImage = createEmptyImage(inputImage.width, inputImage.height);

  double startTime;
  double endTime;

  startTime = getCurrentTime();
  createThreads();

  for (i = 0; i < numberOfThreads; i++) {
    pthread_join(threads[i], (void **)&threadsTimes[i]);
  }
  endTime = getCurrentTime();

  printf("Whole operation time:\t%lf\n", endTime - startTime);
  for (i = 0; i < numberOfThreads; i++) {
    printf("Thread %i time:\t\t%lf\n", i + 1, *threadsTimes[i]);
  }

  saveImage(outputFileName, &outputImage);

  return 0;
}

void cleanExit() {
  if (threads != NULL)
    free(threads);
  if (threadsTimes != NULL) {
    int i = 0;
    for (; i < numberOfThreads; i++)
      free(threadsTimes[i]);
    free(threadsTimes);
  }

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
    filterArgs[i].filter = &filter;
    filterArgs[i].threadId = i;
    if (mode == BLOCK)
      pthread_create(&threads[i], NULL, blockFilter, &filterArgs[i]);
    else if (mode == INTERLEAVED)
      pthread_create(&threads[i], NULL, interleavedFilter, &filterArgs[i]);
    else
      MESSAGE_EXIT("Unkonown mode");
  }
}

void transformColumn(struct FILTER_ARGS *args, int columnId) {
  int y, i, j;
  int c = args->filter->size;
  int mc, mr;
  float s;
  for (y = 0; y < args->inputImage->height; y++) {
    s = 0.0;
    for (i = 0; i < c; i++) {
      mc = max(0, columnId - ceilDiv(c, 2) + i - 1);
      if (mc >= args->inputImage->width)
        mc = args->inputImage->width - 1;
      for (j = 0; j < c; j++) {
        mr = max(0, y - ceilDiv(c, 2) + j - 1);
        if (mr >= args->inputImage->width)
          mr = args->inputImage->height - 1;
        s += args->inputImage->data[mc][mr] * args->filter->data[i][j];
      }
    }
    args->outputImage->data[columnId][y] = _round(s > 0 ? s : 0);
  }
}

void *blockFilter(void *args) {
  double startTime = getCurrentTime();
  struct FILTER_ARGS *filterArgs = args;
  int i;
  int min = filterArgs->threadId * ceilDiv(filterArgs->outputImage->width,
                                           filterArgs->numberOfThreads);
  int max = (filterArgs->threadId + 1) * ceilDiv(filterArgs->outputImage->width,
                                                 filterArgs->numberOfThreads);
  if (max > filterArgs->outputImage->width)
    max = filterArgs->outputImage->width;

  for (i = min; i < max; i++) {
    transformColumn(filterArgs, i);
  }
  double *time = malloc(sizeof(double));
  *time = getCurrentTime() - startTime;
  pthread_exit(time);
}

void *interleavedFilter(void *args) {
  double startTime = getCurrentTime();
  struct FILTER_ARGS *filterArgs = args;
  int i;
  for (i = filterArgs->threadId; i < filterArgs->inputImage->width;
       i += filterArgs->numberOfThreads) {
    transformColumn(filterArgs, i);
  }
  double *time = malloc(sizeof(double));
  *time = getCurrentTime() - startTime;
  pthread_exit(time);
}
