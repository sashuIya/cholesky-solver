#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <stddef.h>

/**
 * Calculates the total number of elements in a symmetric matrix
 * stored in packed upper-triangular format.
 */
static inline size_t get_symmetric_matrix_size(int n) {
  if (n <= 0) return 0;
  return (size_t)n * (n + 1) / 2;
}

/**
 * Calculates the index of the element at (row, col) in a symmetric
 * matrix stored in packed upper-triangular format.
 * Assumes row <= col.
 */
static inline size_t get_symmetric_index(int row, int col, int n) {
  // Formula: sum of lengths of previous rows + offset in current row
  // sum_{p=0}^{row-1} (n-p) + (col-row)
  // = row*n - row*(row-1)/2 + col - row
  return (size_t)row * n - (size_t)row * (row - 1) / 2 + (col - row);
}

#endif
