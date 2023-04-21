/**
 * @file solve.c
 * @brief A Simple TSP Solver.
 * @author aurelien.esnard@u-bordeaux.fr
 * @copyright University of Bordeaux. All rights reserved, 2021.
 *
 **/

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "tsp.h"

#define DISTMAX 10

/* ************************************************************************** */

void usage(int argc, char *argv[]) {
  printf("Usage: %s <options>\n", argv[0]);
  printf(" -l filename: load distance matrix [required]\n");
  printf(" -f first: set first city [default: 0]\n");
  printf(" -v: enable verbose mode\n");
  printf(" -d: enable debug mode\n");
  printf(" -o: enable solver optimization\n");
  printf(" -h: print usage\n");
  exit(EXIT_FAILURE);
}

/* ************************************************************************** */

int main(int argc, char *argv[]) {
  unsigned char options = 0;
  uint first = 0; /* first city */
  char *filename = NULL;
  int c;
  while ((c = getopt(argc, argv, "vdhol:f:")) != -1) {
    if (c == 'f') first = atoi(optarg);
    if (c == 'l') filename = optarg;
    if (c == 'v') options |= VERBOSE;
    if (c == 'd') options |= (VERBOSE | DEBUG);
    if (c == 'o') options |= OPTIMIZE;
    if (c == 'h') usage(argc, argv);
  }
  if (!filename) usage(argc, argv);

  /* create distance matrix */
  uint size = 0;
  uint *distmat = distmat_load(filename, &size);

  /* check arguments */
  assert(distmat);
  assert(size >= 2 && size <= 26); /* city names in range [A,Z] */
  assert(first >= 0 && first < size);

  /* run solver */
  TSP *tsp = tsp_new(size, first, distmat, options);
  uint count = 0;
  printf("TSP problem of size %u starting from city %c.\n", size, 'A' + first);
  distmat_print(size, distmat);
  printf("Starting path exploration...\n");
  path *sol = tsp_solve(tsp, &count);
  printf("TSP solved after %u paths fully explored.\n", count);
  path_print(sol);
  tsp_free(tsp);
  free(distmat);

  return EXIT_SUCCESS;
}

/* ************************************************************************** */
