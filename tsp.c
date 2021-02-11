/**
 * @file tsp.c
 * @brief A Simple TSP Solver.
 * @author aurelien.esnard@u-bordeaux.fr
 * @copyright University of Bordeaux. All rights reserved, 2021.
 *
 **/

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* ************************************************************************** */
/*                                STRUCT                                      */
/* ************************************************************************** */

typedef unsigned int uint;

enum { VERBOSE = 1, OPTIMIZE = 2 };

typedef struct {
  uint size;             /* nb of cities (problem size)) */
  uint seed;             /* random seed */
  uint first;            /* first city */
  uint distmax;          /* max distance bewteen cities */
  uint *distmat;         /* distance matrix */
  unsigned char options; /* options: verbose, optimize */
} TSP;

typedef struct {
  uint *cities; /* array of cities in path */
  uint length;  /* current length of path */
  uint max;     /* max length of path */
  uint dist;    /* current distance of path */
} path;

/* ************************************************************************** */
/*                                MISC                                        */
/* ************************************************************************** */

unsigned long factorial(unsigned long x) {
  if (x <= 1) return 1;
  return x * factorial(x - 1);
}

/* ************************************************************************** */

uint getDist(TSP *tsp, uint first, uint second) {
  assert(tsp);
  assert(first < tsp->size && second < tsp->size);
  return tsp->distmat[first * tsp->size + second];
}

/* ************************************************************************** */
/*                                PATH                                        */
/* ************************************************************************** */

path *createPath(uint max) {
  assert(max > 0);
  path *p = malloc(sizeof(path));
  assert(p);
  p->max = max;
  p->length = 0;
  p->dist = 0;
  p->cities = calloc(max, sizeof(uint));
  assert(p->cities);
  return p;
}

/* ************************************************************************** */

void assignPath(path *src, path *dst) {
  assert(src && dst);
  dst->max = src->max;
  dst->length = src->length;
  dst->dist = src->dist;
  for (uint i = 0; i < dst->length; i++) dst->cities[i] = src->cities[i];
}

/* ************************************************************************** */

void freePath(path *p) {
  if (p) free(p->cities);
  free(p);
}

/* ************************************************************************** */

void printPath(path *p) {
  assert(p);
  printf("[ ");
  for (uint i = 0; i < p->length; i++) printf("%c ", 'A' + p->cities[i]);
  printf("] => (%u)\n", p->dist);
}

/* ************************************************************************** */

uint lastCity(path *p) {
  assert(p);
  assert(p->length > 0);
  return p->cities[p->length - 1];
}

/* ************************************************************************** */

void updatePathDist(TSP *tsp, path *p) {
  uint distsum = 0;
  for (uint i = 0; i < p->length - 1; i++) {
    int first = p->cities[i];
    int second = p->cities[i + 1];
    uint dist = tsp->distmat[first * tsp->size + second];
    distsum += dist;
  }
  p->dist = distsum;
}

/* ************************************************************************** */

void pushCity(TSP *tsp, path *p, uint city) {
  assert(p);
  assert(p->length < p->max);
  assert(city < p->max);
  p->cities[p->length] = city;
  p->length++;
  updatePathDist(tsp, p);
}

/* ************************************************************************** */

void popCity(TSP *tsp, path *p) {
  assert(p);
  assert(p->length > 0);
  p->length--;
  updatePathDist(tsp, p);
}

/* ************************************************************************** */

bool checkPath(TSP *tsp, path *cur, path *sol) {
  assert(cur);
  /* check if current path is invalid */
  if (cur->length <= 1) return true;
  uint last = cur->cities[cur->length - 1];
  for (uint i = 0; i < cur->length - 1; i++) {
    if (cur->cities[i] == last) return false; /* already used */
  }

  /* check if current path is worst than current solution */
  if (tsp->options & OPTIMIZE) {
    if (sol && cur->dist >= sol->dist) return false;
  }

  return true;
}

/* ************************************************************************** */
/*                                   TSP                                      */
/* ************************************************************************** */

void createDistMat(TSP *tsp) {
  assert(tsp);
  srand(tsp->seed);
  uint *distmat = calloc(tsp->size * tsp->size, sizeof(uint));
  assert(distmat);
  for (uint i = 0; i < tsp->size; i++)
    for (uint j = 0; j < i; j++) {
      uint dist = (uint)(rand() % tsp->distmax) + 1; /* random distance in range [1,distmax] */
      distmat[i * tsp->size + j] = distmat[j * tsp->size + i] = dist;
    }
  tsp->distmat = distmat;
}

/* ************************************************************************** */

/* generate a random instance of TSP problem */
TSP *createTSP(uint size, uint first, uint distmax, uint seed, unsigned char options) {
  assert(size >= 2);
  assert(first < size);
  TSP *tsp = malloc(sizeof(TSP));
  assert(tsp);
  tsp->size = size;
  tsp->first = first;
  tsp->distmax = distmax;
  tsp->seed = seed;
  tsp->options = options;
  createDistMat(tsp);
  return tsp;
}

/* ************************************************************************** */

void freeTSP(TSP *p) {
  if (p) free(p->distmat);
  free(p);
}

/* ************************************************************************** */
/*                                 SOLVER                                     */
/* ************************************************************************** */

void solveRec(TSP *tsp, path *cur, path *opt, uint *count) {
  assert(tsp);
  if (cur->length == tsp->size) return;
  /* try to extend the current path with all cities */
  for (uint city = 0; city < tsp->size; city++) {
    pushCity(tsp, cur, city);
    if (checkPath(tsp, cur, opt)) {
      if (cur->length == tsp->size) {
        if (cur->dist < opt->dist) assignPath(cur, opt);
        if (tsp->options & VERBOSE) printPath(cur);
        (*count)++;
      }
      solveRec(tsp, cur, opt, count);
    }
    popCity(tsp, cur);
  }
}

/* ************************************************************************** */

path *solveTSP(TSP *tsp, uint *count) {
  assert(tsp);
  path *cur = createPath(tsp->size);
  path *sol = createPath(tsp->size);
  sol->dist = UINT_MAX;
  pushCity(tsp, cur, tsp->first);
  solveRec(tsp, cur, sol, count);
  freePath(cur);
  return sol;
}

/* ************************************************************************** */
/*                                   MAIN                                     */
/* ************************************************************************** */

void usage(int argc, char *argv[]) {
  printf("Usage: %s [[-vo] size]\n", argv[0]);
  exit(EXIT_FAILURE);
}

/* ************************************************************************** */

int main(int argc, char *argv[]) {
  /* parse args */
  unsigned char options = 0;
  uint size = 5; /* default size */
  int c;
  while ((c = getopt(argc, argv, "vo")) != -1) {
    if (c == 'v') options |= VERBOSE;
    if (c == 'o') options |= OPTIMIZE;
  }
  if (argc - optind == 1) size = atoi(argv[optind]);
  if (argc - optind > 1) usage(argc, argv);

  /* run solver */
  TSP *tsp = createTSP(size, 0, 10, 0, options);
  uint count = 0;
  printf("TSP problem of size %u starting from city %c.\n", tsp->size, 'A' + tsp->first);
  path *sol = solveTSP(tsp, &count);
  uint max = printf("TSP solved after %u paths fully explored over %lu!\n", count, factorial(tsp->size - 1));
  printPath(sol);
  freeTSP(tsp);

  return EXIT_SUCCESS;
}

/* ************************************************************************** */
