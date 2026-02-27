#ifndef ARRAY_OP_H
#define ARRAY_OP_H

#include "matrix_utils.h"

/**
 * @brief Performs the block Cholesky decomposition A = R^T D R.
 * 
 * @param matrix The matrix structure containing data and metadata.
 * @param workspace Pre-allocated workspace for block operations.
 * @return int 0 on success, -1 if the matrix is singular or not positive definite.
 */
int cholesky(CholeskyMatrix* matrix, double* workspace);

/**
 * @brief Solves the system R^T y = b using forward substitution.
 * 
 * @param matrix The decomposed matrix structure.
 * @param rhs The right-hand side vector (replaced by solution y).
 * @param workspace Pre-allocated workspace for block operations.
 * @return int 0 on success, non-zero on error.
 */
int solve_lower_triangle_matrix_system(const CholeskyMatrix* matrix, double* rhs, double* workspace);

/**
 * @brief Solves the system D R x = y using backward substitution.
 * 
 * @param matrix The decomposed matrix structure (including diagonal D).
 * @param rhs The right-hand side vector y (replaced by solution x).
 * @param workspace Pre-allocated workspace for block operations.
 * @return int 0 on success, non-zero on error.
 */
int solve_upper_triangle_matrix_diagonal_system(const CholeskyMatrix* matrix, double* rhs,
                                                double* workspace);

#endif
