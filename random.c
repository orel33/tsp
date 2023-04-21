
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
  uint seed = 0;
  uint size = 0;
  char *filename = NULL;
  if (argc == 3) {
    size = atoi(argv[1]);
    filename = argv[2];
    seed = (uint)time(NULL); // random seed
  } else if (argc == 4) {
    size = atoi(argv[1]);
    filename = argv[2];
    seed = atoi(argv[3]);
  } else {
    printf("Usage: %s <size> <filename> [<seed>]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  assert(size >= 2);

  srand(seed);
  uint *distmat = distmat_random(size, seed, DISTMAX);
  distmat_print(size, distmat);
  distmat_save(size, distmat, filename);

  free(distmat);
  return EXIT_SUCCESS;
}

