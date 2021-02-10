/**
 * @file tsp.c
 * @brief A Simple TSP Solver.
 * @author aurelien.esnard@u-bordeaux.fr
 * @copyright University of Bordeaux. All rights reserved, 2021.
 *
 **/

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* ************************************************************************** */
/*                                STRUCT                                      */
/* ************************************************************************** */

typedef unsigned int uint;

typedef struct {
  uint size;     /* nb of cities (problem size)) */
  uint seed;     /* random seed */
  uint first;    /* first city */
  uint distmax;  /* max distance bewteen cities */
  uint *distmat; /* distance matrix */
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

uint getDistance(TSP *tsp, uint first, uint second) {
  assert(tsp);
  return tsp->distmat[first * tsp->size + second];
}

/* compute a random instance of TSP */
uint *genDistMat(uint size, uint distmax, uint seed) {
  srand(seed);
  uint *dist = calloc(size * size, sizeof(uint));
  assert(dist);
  for (uint i = 0; i < size; i++)
    for (uint j = 0; j < i; j++)
      dist[i * size + j] = dist[j * size + i] = (uint)(rand() % distmax) + 1; /* random distance in range [1,distmax] */
  return dist;
}

void printDistMat(uint size, uint *distmat) {
  printf("Distance matrix:\n");
  for (uint i = 0; i < size; i++)
    for (uint j = i + 1; j < size; j++) printf("* %c-%c (%u)\n", 'A' + i, 'A' + j, distmat[i * size + j]);
}

uint computePathDistance(uint *path, uint length, uint *distmat, uint size) {
  uint distsum = 0;
  for (uint i = 0; i < length - 1; i++) {
    int from = path[i];
    int to = path[i + 1];
    uint dist = distmat[from * size + to];
    distsum += dist;
  }
  return distsum;
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
  p->cities = calloc(max, sizeof(uint));
  assert(p->cities);
  return p;
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
  printf("] => (%u)\n", p->dist);  // , opt ? '*' : ' ');
}

/* ************************************************************************** */

uint lastCity(path *p) {
  assert(p);
  assert(p->length > 0);
  return p->cities[p->length - 1];
}

/* ************************************************************************** */

void addNewCity(path *p, uint city) {
  assert(p);
  assert(p->length < p->max);
  assert(city < p->max);
  p->cities[p->length] = city;
  p->length++;
}

/* ************************************************************************** */

// void removeLastCity(path *p) {
//   assert(p);
//   if (p->length > 0) p->length--;
// }

/* ************************************************************************** */

/* check if the current path is valid */
bool checkPath(path *p) {
  assert(p);
  if (p->length <= 1) return true;
  uint last = p->cities[p->length - 1];
  for (uint i = 0; i < p->length - 1; i++) {
    if (p->cities[i] == last) return false; /* already used */
  }
  return true;
}

/* ************************************************************************** */
/*                                   TSP                                      */
/* ************************************************************************** */

/* generate a random instance of TSP problem */
TSP *createTSP(uint size, uint first, uint distmax, uint seed) {
  assert(size >= 2);
  assert(first < size);
  TSP *tsp = malloc(sizeof(TSP));
  assert(tsp);
  tsp->size = size;
  tsp->first = first;
  tsp->distmax = distmax;
  tsp->seed = seed;
  tsp->distmat = genDistMat(size, distmax, seed);
  assert(tsp->distmat);
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

void solveRec(TSP *tsp, path *p, uint *count) {
  assert(tsp);
  if (p->length == tsp->size) return;
  uint lastcity = lastCity(p);
  for (uint newcity = 0; newcity < tsp->size; newcity++) {
    addNewCity(p, newcity);
    // uint dist = getDistance(tsp, lastcity, newcity);
    if (checkPath(p)) {
      if (p->length == tsp->size) {
        (*count)++;
        printPath(p);
      }
      solveRec(tsp, p, count);
    }
    p->length--; /* remove last city */
  }
}

/* ************************************************************************** */

void solveTSP(TSP *tsp, uint *count) {
  assert(tsp);
  path *p = createPath(tsp->size);
  addNewCity(p, tsp->first); /* lets put the first city in path */
  solveRec(tsp, p, count);
  freePath(p);
}

/* ************************************************************************** */
/*                                   MAIN                                     */
/* ************************************************************************** */

int main(int argc, char *argv[]) {
  assert(argc == 2);
  uint size = atoi(argv[1]);
  uint count = 0;
  TSP *tsp = createTSP(size, 0, 10, 0);
  printf("Solve TSP problem of size %u starting from city %c.\n", tsp->size, 'A' + tsp->first);
  solveTSP(tsp, &count);
  freeTSP(tsp);
  printf("TSP solved in 0.00ms (%u paths explored)\n", count);
  return EXIT_SUCCESS;
}
