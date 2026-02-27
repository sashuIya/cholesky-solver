#include "solver_engine.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array_io.h"
#include "array_op.h"
#include "matrix_utils.h"
#include "timer.h"

int run_cholesky_solver(const SolverConfig* config, SolverResults* results) {
  int matrix_size = config->matrix_size;
  int block_size = config->block_size;
  int return_code = 0;

  CholeskyMatrix matrix = {matrix_size, block_size, NULL, NULL};
  double* vector_answer = NULL;
  double* vector = NULL;
  double* exact_rhs = NULL;
  double* rhs = NULL;
  double* workspace = NULL;

  /* 1. Allocation */
  matrix.data = (double*)malloc(get_symmetric_matrix_size(matrix_size) * sizeof(double));
  matrix.diagonal = (double*)malloc(matrix_size * sizeof(double));
  vector_answer = (double*)malloc(matrix_size * sizeof(double));
  vector = (double*)malloc(matrix_size * sizeof(double));
  exact_rhs = (double*)malloc(matrix_size * sizeof(double));
  rhs = (double*)malloc(matrix_size * sizeof(double));
  workspace = (double*)malloc(3 * (size_t)block_size * block_size * sizeof(double));

  if (!matrix.data || !matrix.diagonal || !vector_answer || !vector || !exact_rhs || !rhs ||
      !workspace) {
    return_code = -2;
    goto cleanup;
  }

  memset(matrix.data, 0, get_symmetric_matrix_size(matrix_size) * sizeof(double));
  memset(matrix.diagonal, 0, matrix_size * sizeof(double));
  memset(vector_answer, 0, matrix_size * sizeof(double));
  memset(vector, 0, matrix_size * sizeof(double));
  memset(exact_rhs, 0, matrix_size * sizeof(double));
  memset(rhs, 0, matrix_size * sizeof(double));
  memset(workspace, 0, 3 * (size_t)block_size * block_size * sizeof(double));

  /* 2. Initialization */
  fill_vector_answer(matrix_size, vector_answer);

  if (config->input_file == NULL) {
    if (fill_matrix(&matrix, vector_answer, rhs)) {
      return_code = -3;
      goto cleanup;
    }
  } else {
    if (read_matrix(&matrix, vector_answer, rhs, config->input_file)) {
      return_code = -4;
      goto cleanup;
    }
  }

  for (int i = 0; i < matrix_size; i++) {
    exact_rhs[i] = rhs[i];
    vector[i] = rhs[i];
  }

  print_time("on initialization");

  if (matrix_size < 15) {
    printf("matrix A:\n");
    printf_matrix(&matrix);
    printf("\nrhs:\n");
    for (int i = 0; i < matrix_size; ++i) printf("%.10f ", rhs[i]);
    printf("\n\n");
  }

  /* 3. Algorithm Execution */
  if (cholesky(&matrix, workspace)) {
    return_code = -10;
    goto cleanup;
  }
  print_time("on cholesky decomposition");

  if (solve_lower_triangle_matrix_system(&matrix, vector, workspace)) {
    return_code = -11;
    goto cleanup;
  }

  if (solve_upper_triangle_matrix_diagonal_system(&matrix, vector, workspace)) {
    return_code = -12;
    goto cleanup;
  }

  if (matrix_size < 15) {
    printf("cholesky decomposition:\n");
    printf_matrix(&matrix);
    printf("\ndiagonal:\n");
    for (int i = 0; i < matrix_size; i++) printf("%.1f ", matrix.diagonal[i]);
    printf("\n\n");
  }
  print_time("on algorithm");

  /* 4. Verification */
  double residual = 0, rhs_norm = 0, answer_error = 0;

  // Re-generate/read matrix to verify residual
  if (config->input_file == NULL) {
    fill_matrix(&matrix, vector, rhs);
  } else {
    read_matrix(&matrix, vector, rhs, config->input_file);
  }

  for (int i = 0; i < matrix_size; ++i) {
    residual += (exact_rhs[i] - rhs[i]) * (exact_rhs[i] - rhs[i]);
    rhs_norm += exact_rhs[i] * exact_rhs[i];
    answer_error += (vector_answer[i] - vector[i]) * (vector_answer[i] - vector[i]);
  }

  results->residual = sqrt(residual);
  results->rhs_norm = sqrt(rhs_norm);
  results->answer_error = sqrt(answer_error);

  // Store a small sample of the solution for main to print
  results->solution_sample_size = (matrix_size < 5 ? matrix_size : 5);
  results->solution_sample = (double*)malloc(results->solution_sample_size * sizeof(double));
  if (results->solution_sample) {
    memcpy(results->solution_sample, vector, results->solution_sample_size * sizeof(double));
  }

cleanup:
  if (matrix.data) free(matrix.data);
  if (matrix.diagonal) free(matrix.diagonal);
  if (vector_answer) free(vector_answer);
  if (vector) free(vector);
  if (exact_rhs) free(exact_rhs);
  if (rhs) free(rhs);
  if (workspace) free(workspace);

  return return_code;
}
