#ifndef ARRAY_OP_H
#define ARRAY_OP_H

#include "matrix_utils.h"

int cholesky(CholeskyMatrix* matrix, double* workspace);

int solve_lower_triangle_matrix_system(const CholeskyMatrix* matrix, double* rhs, double* workspace);

int solve_upper_triangle_matrix_diagonal_system(const CholeskyMatrix* matrix, double* rhs,
                                                double* workspace);

#endif
