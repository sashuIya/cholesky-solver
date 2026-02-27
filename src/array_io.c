#include "array_io.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix_utils.h"

int fill_matrix(int n, double* matrix, const double* vector_answer, double* rhs) {
  int i, j;

  for (i = 0; i < n; ++i) rhs[i] = 0;

  for (i = 0; i < n; ++i) {
    for (j = 0; j < i; j++) {
      rhs[i] += matrix[get_symmetric_index(j, i, n)] * vector_answer[j];
    }

    for (j = i; j < n; j++) {
      matrix[get_symmetric_index(i, j, n)] = fabs(n - j);

      rhs[i] += matrix[get_symmetric_index(i, j, n)] * vector_answer[j];
    }
  }

  return 0;
}

int stupid_fill_matrix(int n, double* matrix) {
  int i, j;

  for (i = 0; i < n; ++i)
    for (j = 0; j < n; ++j) matrix[i * n + j] = 1.0 / (i + j + 1.0);

  return 0;
}

int read_matrix(int matrix_size, double** p_a, const double* vector_answer, double* rhs,
                const char* input_file_name) {
  int i, j;
  FILE* input_file;
  double* matrix;
  double tmp;
  matrix = (*p_a);

  input_file = fopen(input_file_name, "r");
  if (input_file == NULL) {
    printf("Error: cannot open input file\n");
    return -1;
  }

  for (i = 0; i < matrix_size; ++i) rhs[i] = 0;

  for (i = 0; i < matrix_size; i++) {
    for (j = 0; j < i; ++j) {
      if (fscanf(input_file, "%lf", &tmp) != 1) {
        printf("Cannot read matrix\n");
        return -2;
      }

      rhs[i] += tmp * vector_answer[j];
    }

    for (j = i; j < matrix_size; j++) {
      if (fscanf(input_file, "%lf", matrix + get_symmetric_index(i, j, matrix_size)) != 1) {
        printf("Cannot read matrix\n");
        return -3;
      }

      rhs[i] += matrix[get_symmetric_index(i, j, matrix_size)] * vector_answer[j];
    }
  }

  fclose(input_file);
  return 0;
}

int stupid_read_matrix(int matrix_size, double** p_a, const char* input_file_name) {
  int i, j;
  FILE* input_file;
  double* matrix;
  matrix = (*p_a);

  input_file = fopen(input_file_name, "r");
  if (input_file == NULL) {
    printf("Error: cannot open input file\n");
    return -1;
  }

  for (i = 0; i < matrix_size; i++) {
    for (j = 0; j < matrix_size; j++)
      if (fscanf(input_file, "%lf", matrix + i * matrix_size + j) != 1) {
        printf("Cannot read matrix\n");
        return -3;
      }
  }

  fclose(input_file);
  return 0;
}

void printf_matrix(int n, const double* matrix) {
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++)
      if (j >= i)
        printf("%4.3lf ", matrix[get_symmetric_index(i, j, n)]);
      else
        printf("%4.3lf ", matrix[get_symmetric_index(j, i, n)]);

    printf("\n");
  }
}

void stupid_printf_matrix(int n, const double* matrix) {
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) printf("%.3lf ", matrix[n * i + j]);

    printf("\n");
  }
}

void fill_vector_answer(int n, double* vector_answer) {
  int i;
  memset(vector_answer, 0, n * sizeof(double));

  for (i = 0; i < n; i += 2) vector_answer[i] = 1;
}
