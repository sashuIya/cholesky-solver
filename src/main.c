#include <stdio.h>
#include <stdlib.h>

#include "solver_engine.h"
#include "timer.h"

int main(int argc, char* argv[]) {
  SolverConfig config = {0, 0, NULL};
  SolverResults results = {0, 0, 0, NULL, 0};
  int return_code = 0;

  timer_start();

  /* 1. Argument Parsing */
  if (argc == 3 || argc == 4) {
    char* endptr;
    config.matrix_size = (int)strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || config.matrix_size <= 0) {
      printf("Error: invalid matrix size '%s'\n", argv[1]);
      return -1;
    }

    config.block_size = (int)strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || config.block_size <= 0 || config.block_size > config.matrix_size) {
      printf("Error: invalid block size '%s' (must be between 1 and %d)\n", argv[2],
             config.matrix_size);
      return -1;
    }

    if (argc == 4) {
      config.input_file = argv[3];
    }
  } else {
    printf("Usage: ./cholesky_solver (matrix_size) (block_size) [matrix_input_file]\n");
    return 0;
  }

  /* 2. Run Solver Engine */
  return_code = run_cholesky_solver(&config, &results);

  /* 3. Result Reporting */
  if (return_code == 0) {
    printf("Answer:\n");
    for (int i = 0; i < results.solution_sample_size; ++i) {
      printf("%.10lf ", results.solution_sample[i]);
    }
    printf("\n\n");

    printf("Error: %11.5le ; Residual: %11.5le (%11.5le)\n", results.answer_error, results.residual,
           results.residual / results.rhs_norm);
  } else {
    printf("Solver failed with error code: %d\n", return_code);
  }

  /* 4. Final Cleanup */
  if (results.solution_sample) free(results.solution_sample);

  return return_code;
}
