/**
 * @file tsp.c
 * @brief A Simple TSP Solver.
 * @author aurelien.esnard@u-bordeaux.fr
 * @copyright University of Bordeaux. All rights reserved, 2023.
 *
 **/

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tsp.h"

/* ************************************************************************** */
/*                                TYPES                                       */
/* ************************************************************************** */

typedef struct TSP {
  uint size;             /* nb of cities (problem size)) */
  uint first;            /* first city */
  uint *distmat;         /* distance matrix */
  unsigned char options; /* options: verbose, debug, optimize, ... */
} TSP;

/* ************************************************************************** */

typedef struct path {
  uint *array; /* array of cities in path */
  uint curlen; /* current length of path */
  uint maxlen; /* max length of path */
  uint dist;   /* current distance of path */
} path;

/* ************************************************************************** */
/*                                    PATH                                    */
/* ************************************************************************** */

path *path_new(uint maxlen, uint dist) {
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

static void path_copy(path *src, path *dst) {
  assert(src && dst);
  dst->maxlen = src->maxlen;
  dst->curlen = src->curlen;
  dst->dist = src->dist;
  for (uint i = 0; i < dst->curlen; i++) dst->array[i] = src->array[i];
}

/* ************************************************************************** */

void path_free(path *p) {
  if (p) free(p->array);
  free(p);
}

/* ************************************************************************** */

void path_print(path *p) {
  assert(p);
  printf("[ ");
  for (uint i = 0; i < p->curlen; i++) printf("%c ", 'A' + p->array[i]);
  for (uint i = p->curlen; i < p->maxlen; i++) printf("- ");
  printf("] => (%u)\n", p->dist);
}

/* ************************************************************************** */

static void path_update_dist(TSP *tsp, path *p) {
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

static void path_push(TSP *tsp, path *p, uint city) {
  assert(p);
  assert(p->curlen < p->maxlen);
  assert(city < p->maxlen);
  p->array[p->curlen] = city;
  p->curlen++;
  path_update_dist(tsp, p);
}

/* ************************************************************************** */

static void path_pop(TSP *tsp, path *p) {
  assert(p);
  assert(p->curlen > 0);
  p->curlen--;
  path_update_dist(tsp, p);
}

/* ************************************************************************** */

static bool path_check(TSP *tsp, path *cur, path *sol) {
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

uint path_dist(path *p) {
  assert(p);
  return p->dist;
}

/* ************************************************************************** */
/*                              DISTANCE MATRIX                               */
/* ************************************************************************** */

uint *distmat_random(uint size, uint seed, uint distmax) {
  srand(seed);
  // FIXME: correct this function
  uint *distmat = calloc(size * size, sizeof(uint)); /* memory set to zero */
  assert(distmat);
  for (uint i = 0; i < size; i++)
    for (uint j = 0; j < i; j++) {
      uint dist = (uint)(rand() % distmax) + 1; /* random distance in range [1,distmax] */
      distmat[i * size + j] = distmat[j * size + i] = dist;
    }
  return distmat;
}

/* ************************************************************************** */

uint *distmat_load(char *filename, uint *size) {
  assert(filename);
  assert(size);
  FILE *file = fopen(filename, "r");
  assert(file);
  fscanf(file, "%u", size);
  assert(*size > 0);
  // printf("Loading distance matrix of size %u from file \"%s\"\n", *size, filename);
  uint *distmat = calloc(*size * *size, sizeof(uint)); /* memory set to zero */
  for (uint i = 0; i < *size; i++)
    for (uint j = 0; j < *size; j++) {
      uint dist;
      fscanf(file, "%u", &dist);
      distmat[i * *size + j] = dist;
    }
  fclose(file);
  return distmat;
}

/* ************************************************************************** */

void distmat_save(uint size, uint *distmat, char *filename) {
  assert(filename);
  assert(size >= 2);
  assert(distmat);
  FILE *file = fopen(filename, "w");
  assert(file);
  fprintf(file, "%u\n", size);
  for (uint i = 0; i < size; i++) {
    for (uint j = 0; j < size; j++) {
      uint dist = distmat[i * size + j];
      fprintf(file, "%u ", dist);
    }
    fprintf(file, "\n");
  }
  fclose(file);
}

/* ************************************************************************** */

void distmat_print(uint size, uint *distmat) {
  assert(size >= 2);
  assert(distmat);
  /* header */
  printf("    ");
  for (uint j = 0; j < size; j++) printf(" %c ", 'A' + j);
  printf("\n");
  /* separator */
  printf("  --");
  for (uint j = 0; j < size; j++) printf("---");
  printf("-\n");
  /* distance matrix */
  for (uint i = 0; i < size; i++) {
    printf("%c | ", 'A' + i);
    for (uint j = 0; j < size; j++) {
      printf("%2u ", distmat[i * size + j]);
    }
    printf("|\n");
  }
  /* separator */
  printf("  --");
  for (uint j = 0; j < size; j++) printf("---");
  printf("-\n");
}

/* ************************************************************************** */
/*                                   TSP                                      */
/* ************************************************************************** */

TSP *tsp_new(uint size, uint first, uint *distmat, unsigned char options) {
  assert(size >= 2);
  assert(first < size);
  assert(distmat);
  TSP *tsp = malloc(sizeof(TSP));
  assert(tsp);
  tsp->size = size;
  tsp->first = first;
  tsp->options = options;
  tsp->distmat = distmat;
  return tsp;
}

/* ************************************************************************** */

void tsp_free(TSP *tsp) { free(tsp); }

/* ************************************************************************** */

static void tsp_solve_rec(TSP *tsp, path *cur, path *sol, uint *count) {
  assert(tsp);
  if (cur->curlen == tsp->size) return;
  if (tsp->options & DEBUG) path_print(cur);
  /* try to extend the current path with all cities */
  for (uint city = 0; city < tsp->size; city++) {
    path_push(tsp, cur, city);
    if (path_check(tsp, cur, sol)) {
      if (cur->curlen == tsp->size) {
        path_push(tsp, cur, tsp->first); /* come back to the first city */
        if (cur->dist < sol->dist) path_copy(cur, sol);
        if (tsp->options & VERBOSE) path_print(cur);
        if (count) (*count)++;
        path_pop(tsp, cur);
      }
      tsp_solve_rec(tsp, cur, sol, count);
    }
    path_pop(tsp, cur);
  }
}

/* ************************************************************************** */

path *tsp_solve(TSP *tsp, uint *count) {
  assert(tsp);
  path *cur = path_new(tsp->size + 1, 0);
  path *sol = path_new(tsp->size + 1, UINT_MAX);
  path_push(tsp, cur, tsp->first);
  tsp_solve_rec(tsp, cur, sol, count);
  path_free(cur);
  return sol;
}

/* ************************************************************************** */
