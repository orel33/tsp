
/**
 * @file random.c
 * @brief Random Distance Matrix Generator.
 * @author aurelien.esnard@u-bordeaux.fr
 * @copyright University of Bordeaux. All rights reserved, 2023.
 *
 **/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tsp.h"

#define DISTMAX 10

int main(int argc, char *argv[]) {
  if (argc > 4) {
    printf("Usage: %s [<size>] [<filename>] [<seed>]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  uint size = 5;
  uint seed = (uint)time(NULL);  // random seed
  char *filename = NULL;

  if (argc >= 2) size = atoi(argv[1]);
  if (argc >= 3) filename = argv[2];
  if (argc == 4) seed = atoi(argv[3]);
  assert(size >= 2);

  srand(seed);
  uint *distmat = distmat_random(size, seed, DISTMAX);
  distmat_print(size, distmat);
  if (filename) distmat_save(size, distmat, filename);

  free(distmat);
  return EXIT_SUCCESS;
}
