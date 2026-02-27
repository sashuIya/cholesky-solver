#ifndef ARRAY_OP_H
#define ARRAY_OP_H

#include "matrix_utils.h"

// Performs the block Cholesky decomposition A = R^T D R.
//
// Args:
//   matrix: Pointer to the CholeskyMatrix structure.
//   workspace: Pre-allocated memory for intermediate block operations.
//
// Returns:
//   0 on success, -1 if the matrix is singular or not positive definite.
int cholesky(CholeskyMatrix* matrix, double* workspace);

// Solves the system R^T y = b using forward substitution.
//
// Args:
//   matrix: Decomposed matrix structure.
//   rhs: The right-hand side vector (modified in-place to solution y).
//   workspace: Pre-allocated block workspace.
//
// Returns:
//   0 on success, non-zero on error.
int solve_lower_triangle_matrix_system(const CholeskyMatrix* matrix, double* rhs,
                                       double* workspace);

// Solves the system D R x = y using backward substitution.
//
// Args:
//   matrix: Decomposed matrix structure (including diagonal D).
//   rhs: The right-hand side vector y (modified in-place to solution x).
//   workspace: Pre-allocated block workspace.
//
// Returns:
//   0 on success, non-zero on error.
int solve_upper_triangle_matrix_diagonal_system(const CholeskyMatrix* matrix, double* rhs,
                                                double* workspace);

#endif
