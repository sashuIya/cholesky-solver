#ifndef ARRAY_IO_H
#define ARRAY_IO_H

#include "matrix_utils.h"

// Fills the matrix with test data and calculates the matching RHS for a known answer.
//
// Args:
//   matrix: Pointer to the matrix structure to fill.
//   vector_answer: The known exact solution vector for error calculation.
//   rhs: Output buffer for the resulting right-hand side vector.
//
// Returns:
//   0 on success.
int fill_matrix(CholeskyMatrix* matrix, const double* vector_answer, double* rhs);

// Reads the matrix from a file and calculates the matching RHS for a known answer.
//
// Args:
//   matrix: Pointer to the matrix structure to fill.
//   vector_answer: The known exact solution vector.
//   rhs: Output buffer for the resulting right-hand side vector.
//   input_file_name: Path to the matrix file.
//
// Returns:
//   0 on success, non-zero on error.
int read_matrix(CholeskyMatrix* matrix, const double* vector_answer, double* rhs,
                const char* input_file_name);

// Prints the matrix to standard output.
//
// Args:
//   matrix: Pointer to the matrix structure to print.
void printf_matrix(const CholeskyMatrix* matrix);

// Fills a vector with a known pattern (alternating 1s and 0s) to serve as x_exact.
//
// Args:
//   n: Dimension of the vector.
//   vector_answer: Buffer to be filled.
void fill_vector_answer(int n, double* vector_answer);

#endif
