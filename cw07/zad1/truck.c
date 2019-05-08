#include "argsProcessor.h"
#include "load.h"

int truckMaxLoad, maxNumberOfLoads, maxSummedWeightOfLoad;

struct Load * loadOnTruck;

int main(int argc, char** argv) {
  truckMaxLoad = getArgAsInt(argv, 1);
  maxNumberOfLoads = getArgAsInt(argv, 2);
  maxSummedWeightOfLoad = getArgAsInt(argv, 3);
  printf("%i, %i, %i\n", truckMaxLoad, maxNumberOfLoads, maxSummedWeightOfLoad);
  return 0;
}
