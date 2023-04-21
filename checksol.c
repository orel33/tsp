
/**
 * @file checksol.c
 * @brief Check solution.
 * @author aurelien.esnard@u-bordeaux.fr
 * @copyright University of Bordeaux. All rights reserved, 2023.
 *
 **/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "tsp.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <filename> <mindist>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char *filename = argv[1];
  uint mindist = atoi(argv[2]);
  uint options = 0;
  uint size;
  uint first = 0; /* first city */
  uint *distmat = distmat_load(filename, &size);
  assert(size >= 2);

  TSP *tsp = tsp_new(size, first, distmat, options);
  distmat_print(size, distmat);
  uint count = 0;
  path *sol = tsp_solve(tsp, &count);
  path_print(sol);
  uint dist = path_dist(sol);
  printf("tsp dist: %u (expected: %u)\n", dist, mindist);
  tsp_free(tsp);
  free(distmat);
  if (dist != mindist) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
