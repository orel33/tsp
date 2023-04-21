/**
 * @file tsp.h
 * @brief A Simple TSP Solver.
 * @author aurelien.esnard@u-bordeaux.fr
 * @copyright University of Bordeaux. All rights reserved, 2023.
 *
 **/

#ifndef TSP_H
#define TSP_H

/* ************************************************************************** */
/*                                 TYPES                                      */
/* ************************************************************************** */

typedef unsigned int uint;
enum { NONE = 0, VERBOSE = 1, DEBUG = 2, OPTIMIZE = 4 };
typedef struct TSP TSP;
typedef struct path path;

/* ************************************************************************** */
/*                                    PATH                                    */
/* ************************************************************************** */

/**
 * @brief Print a path.
 * @param p path
 */
void path_print(path *p);

/**
 * @brief Compute the distance of a path.
 * @param p path
 * @return uint distance
 */
uint path_dist(path *p);

/* ************************************************************************** */
/*                              DISTANCE MATRIX                               */
/* ************************************************************************** */

/**
 * @brief Create a random distance matrix.
 *
 * @param size problem size
 * @param seed random seed
 * @param distmax max distance
 * @return uint* distance matrix
 */
uint *distmat_random(uint size, uint seed, uint distmax);

/**
 * @brief Print a distance matrix.
 *
 * @param size problem size
 * @param distmat distance matrix
 */
void distmat_print(uint size, uint *distmat);

/**
 * @brief Load a distance matrix from a file.
 *
 * @param filename filename
 * @param size problem size (output)
 * @return uint* distance matrix
 */
uint *distmat_load(char *filename, uint *size);

/**
 * @brief Save a distance matrix to a file.
 * @param size problem size
 * @param distmat distance matrix
 * @param filename filename
 */
void distmat_save(uint size, uint *distmat, char *filename);

/* ************************************************************************** */
/*                                    TSP                                     */
/* ************************************************************************** */

/**
 * @brief Create a new TSP instance.
 * @param size problem size
 * @param first first city
 * @param distmat distance matrix
 * @param options options: verbose, debug, optimize, ...
 */
TSP *tsp_new(uint size, uint first, uint *distmat, unsigned char options);

/**
 * @brief Solve the TSP problem.
 *
 * @param tsp  TSP instance
 * @param count  number of solutions
 * @return path*  array of solutions
 */
path *tsp_solve(TSP *tsp, uint *count);

/**
 * @brief Free a TSP instance.
 *
 * @param tsp  TSP instance
 */
void tsp_free(TSP *tsp);

/* ************************************************************************** */

#endif
