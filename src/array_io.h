#ifndef ARRAY_IO_H
#define ARRAY_IO_H

int fill_matrix(int n, double* matrix, const double* vector_answer, double* rhs);
int stupid_fill_matrix(int n, double* matrix);
int read_matrix(int matrix_size, double** p_a, const double* vector_answer, double* rhs,
                const char* input_file_name);
int stupid_read_matrix(int matrix_size, double** p_a, const char* input_file_name);
void printf_matrix(int n, const double* matrix);
void stupid_printf_matrix(int n, const double* matrix);
void fill_vector_answer(int n, double* vector_answer);

#endif
