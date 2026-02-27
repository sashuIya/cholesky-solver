#ifndef SOLVER_ENGINE_H
#define SOLVER_ENGINE_H

#include "matrix_utils.h"

// Configuration for the Cholesky solver execution.
typedef struct {
  int matrix_size;    // Total dimension of the symmetric matrix.
  int block_size;     // Size of square blocks for cache optimization.
  const char* input_file; // Optional file path to read matrix from (NULL for auto-fill).
} SolverConfig;

// Results and metrics from the solver execution.
typedef struct {
  double residual;     // The L2 norm of (Ax - b).
  double answer_error; // The L2 norm of (x - x_exact).
  double rhs_norm;     // The L2 norm of the right-hand side vector b.
  double* solution_sample;  // Pointer to a sample of the solution vector.
  int solution_sample_size; // Number of elements in the solution sample.
} SolverResults;

// Orchestrates the full Cholesky solving process.
//
// Performs allocation, initialization, Cholesky decomposition,
// forward/backward substitution, and solution verification.
//
// Args:
//   config: Configuration for the solver (sizes, input files).
//   results: Output structure to store residual, error, and solution sample.
//
// Returns:
//   0 on success, or a non-zero error code on failure.
int run_cholesky_solver(const SolverConfig* config, SolverResults* results);

#endif
