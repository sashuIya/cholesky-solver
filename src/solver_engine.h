#ifndef SOLVER_ENGINE_H
#define SOLVER_ENGINE_H

#include "matrix_utils.h"

typedef struct {
  int matrix_size;
  int block_size;
  const char* input_file;
} SolverConfig;

typedef struct {
  double residual;
  double answer_error;
  double rhs_norm;
  double* solution_sample;  // Pointer to the first few elements of the solution
  int solution_sample_size;
} SolverResults;

/**
 * Orchestrates the full Cholesky solving process:
 * Allocation -> Initialization -> Decomposition -> Solving -> Verification.
 * Returns 0 on success, non-zero error code on failure.
 */
int run_cholesky_solver(const SolverConfig* config, SolverResults* results);

#endif
