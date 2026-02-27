#ifndef ARRAY_OP_H
#define ARRAY_OP_H

int cholesky(int matrix_size, double* matrix, double* diagonal, double* workspace, int block_size);

int solve_lower_triangle_matrix_system(int matrix_size, const double* matrix, double* rhs,
                                       double* workspace, int block_size);

int solve_upper_triangle_matrix_diagonal_system(int matrix_size, const double* matrix,
                                                const double* diagonal, double* rhs,
                                                double* workspace, int block_size);

#endif
