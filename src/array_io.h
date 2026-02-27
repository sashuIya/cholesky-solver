#ifndef ARRAY_IO_H
#define ARRAY_IO_H

#include "matrix_utils.h"

/**
 * @brief Fills the matrix with test data and calculates the matching RHS for a known answer.
 * 
 * @param matrix The matrix structure to fill.
 * @param vector_answer The known exact solution vector.
 * @param rhs The resulting right-hand side vector.
 * @return int 0 on success.
 */
int fill_matrix(CholeskyMatrix* matrix, const double* vector_answer, double* rhs);

/**
 * @brief Reads the matrix from a file and calculates the matching RHS for a known answer.
 * 
 * @param matrix The matrix structure to fill.
 * @param vector_answer The known exact solution vector.
 * @param rhs The resulting right-hand side vector.
 * @param input_file_name Path to the matrix file.
 * @return int 0 on success, non-zero on error.
 */
int read_matrix(CholeskyMatrix* matrix, const double* vector_answer, double* rhs,
                const char* input_file_name);

/**
 * @brief Prints the matrix to standard output.
 * 
 * @param matrix The matrix structure to print.
 */
void printf_matrix(const CholeskyMatrix* matrix);

/**
 * @brief Fills a vector with a alternating 1s and 0s to serve as a known exact solution.
 * 
 * @param n Dimension of the vector.
 * @param vector_answer Pointer to the vector to fill.
 */
void fill_vector_answer(int n, double* vector_answer);

#endif
