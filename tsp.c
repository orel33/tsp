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

#define DISTMAX 10

/* ************************************************************************** */
/*                                STRUCT                                      */
/* ************************************************************************** */

typedef unsigned int uint;

enum { VERBOSE = 1, DEBUG = 2, OPTIMIZE = 4 };

typedef struct {
  uint size;             /* nb of cities (problem size)) */
  uint seed;             /* random seed */
  uint first;            /* first city */
  uint distmax;          /* max distance bewteen cities */
  uint *distmat;         /* distance matrix */
  unsigned char options; /* options: verbose, debug, optimize, ... */
} TSP;

typedef struct {
  uint *array; /* array of cities in path */
  uint curlen; /* current length of path */
  uint maxlen; /* max length of path */
  uint dist;   /* current distance of path */
} path;

/* ************************************************************************** */
/*                                MISC                                        */
/* ************************************************************************** */

unsigned long long factorial(unsigned long long x) {
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

path *createPath(uint maxlen, uint dist) {
  assert(maxlen > 0);
  path *p = malloc(sizeof(path));
  assert(p);
  p->maxlen = maxlen;
  p->curlen = 0;
  p->dist = dist;
  p->array = calloc(maxlen, sizeof(uint));
  assert(p->array);
  return p;
}

/* ************************************************************************** */

void assignPath(path *src, path *dst) {
  assert(src && dst);
  dst->maxlen = src->maxlen;
  dst->curlen = src->curlen;
  dst->dist = src->dist;
  for (uint i = 0; i < dst->curlen; i++) dst->array[i] = src->array[i];
}

/* ************************************************************************** */

void freePath(path *p) {
  if (p) free(p->array);
  free(p);
}

/* ************************************************************************** */

void printPath(path *p) {
  assert(p);
  printf("[ ");
  for (uint i = 0; i < p->curlen; i++) printf("%c ", 'A' + p->array[i]);
  for (uint i = p->curlen; i < p->maxlen; i++) printf("- ");
  printf("] => (%u)\n", p->dist);
}

/* ************************************************************************** */

uint lastCity(path *p) {
  assert(p);
  assert(p->curlen > 0);
  return p->array[p->curlen - 1];
}

/* ************************************************************************** */

void updatePathDist(TSP *tsp, path *p) {
  uint distsum = 0;
  for (uint i = 0; i < p->curlen - 1; i++) {
    int first = p->array[i];
    int second = p->array[i + 1];
    uint dist = tsp->distmat[first * tsp->size + second];
    distsum += dist;
  }
  p->dist = distsum;
}

/* ************************************************************************** */

void pushCity(TSP *tsp, path *p, uint city) {
  assert(p);
  assert(p->curlen < p->maxlen);
  assert(city < p->maxlen);
  p->array[p->curlen] = city;
  p->curlen++;
  updatePathDist(tsp, p);
}

/* ************************************************************************** */

void popCity(TSP *tsp, path *p) {
  assert(p);
  assert(p->curlen > 0);
  p->curlen--;
  updatePathDist(tsp, p);
}

/* ************************************************************************** */

bool checkPath(TSP *tsp, path *cur, path *sol) {
  assert(cur);
  /* check if current path is invalid */
  if (cur->curlen <= 1) return true;
  uint last = cur->array[cur->curlen - 1];
  for (uint i = 0; i < cur->curlen - 1; i++) {
    if (cur->array[i] == last) return false; /* already used */
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

void printDistMat(TSP *tsp) {
  assert(tsp);
  /* header */
  printf("    ");
  for (uint j = 0; j < tsp->size; j++) printf(" %c ", 'A' + j);
  printf("\n");
  /* separator */
  printf("  --");
  for (uint j = 0; j < tsp->size; j++) printf("---");
  printf("-\n");
  /* distance matrix */
  for (uint i = 0; i < tsp->size; i++) {
    printf("%c | ", 'A' + i);
    for (uint j = 0; j < tsp->size; j++) {
      printf("%2u ", tsp->distmat[i * tsp->size + j]);
    }
    printf("|\n");
  }
  /* separator */
  printf("  --");
  for (uint j = 0; j < tsp->size; j++) printf("---");
  printf("-\n");
}

/* ************************************************************************** */

/* generate a random instance of TSP problem */
TSP *createTSP(uint size, uint first, uint seed, unsigned char options) {
  assert(size >= 2);
  assert(first < size);
  TSP *tsp = malloc(sizeof(TSP));
  assert(tsp);
  tsp->size = size;
  tsp->first = first;
  tsp->distmax = DISTMAX;
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

void solveRec(TSP *tsp, path *cur, path *sol, uint *count) {
  assert(tsp);
  if (cur->curlen == tsp->size) return;
  if (tsp->options & DEBUG) printPath(cur);
  /* try to extend the current path with all cities */
  for (uint city = 0; city < tsp->size; city++) {
    pushCity(tsp, cur, city);
    if (checkPath(tsp, cur, sol)) {
      if (cur->curlen == tsp->size) {
        pushCity(tsp, cur, tsp->first); /* come back to the first city */
        if (cur->dist < sol->dist) assignPath(cur, sol);
        if (tsp->options & VERBOSE) printPath(cur);
        (*count)++;
        popCity(tsp, cur);
      }
      solveRec(tsp, cur, sol, count);
    }
    popCity(tsp, cur);
  }
}

/* ************************************************************************** */

path *solveTSP(TSP *tsp, uint *count) {
  assert(tsp);
  path *cur = createPath(tsp->size + 1, 0);
  path *sol = createPath(tsp->size + 1, UINT_MAX);
  pushCity(tsp, cur, tsp->first);
  solveRec(tsp, cur, sol, count);
  freePath(cur);
  return sol;
}

/* ************************************************************************** */
/*                                   MAIN                                     */
/* ************************************************************************** */

void usage(int argc, char *argv[]) {
  printf("Usage: %s -n <size> [<options>]\n", argv[0]);
  printf(" -n size: set problem size >=2 [required]\n");
  printf(" ---------- options ----------\n");
  printf(" -f first: set first city [default: 0]\n");
  printf(" -s seed: set random seed [default: 0]\n");
  printf(" -o: enable solver optimization\n");
  printf(" -v: enable verbose mode\n");
  printf(" -d: enable debug mode\n");
  printf(" -h: print usage\n");
  exit(EXIT_FAILURE);
}

/* ************************************************************************** */

int main(int argc, char *argv[]) {
  /* parse args */
  unsigned char options = 0;
  uint size = 0;  /* problem size */
  uint seed = 0;  /* default seed */
  uint first = 0; /* first city */
  int c;
  while ((c = getopt(argc, argv, "vdhon:s:f:")) != -1) {
    if (c == 'v') options |= VERBOSE;
    if (c == 'd') options |= (VERBOSE | DEBUG);
    if (c == 'o') options |= OPTIMIZE;
    if (c == 's') seed = atoi(optarg);
    if (c == 'n') size = atoi(optarg);
    if (c == 'f') first = atoi(optarg);
    if (c == 'h') usage(argc, argv);
  }
  if (size == 0) usage(argc, argv);
  assert(size >= 2 && size <= 26); /* city names in range [A,Z] */
  assert(first >= 0 && first < size);

  /* run solver */
  TSP *tsp = createTSP(size, first, seed, options);
  uint count = 0;
  printf("TSP problem of size %u starting from city %c (seed %u).\n", tsp->size, 'A' + tsp->first, seed);
  printDistMat(tsp);
  printf("Starting path exploration...\n");
  path *sol = solveTSP(tsp, &count);
  unsigned long long total = factorial(tsp->size - 1);
  printf("TSP solved after %u paths fully explored over %llu.\n", count, total);
  printPath(sol);
  freeTSP(tsp);

  return EXIT_SUCCESS;
}

/* ************************************************************************** */
