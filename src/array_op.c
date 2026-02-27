#include "array_op.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix_utils.h"

const double EPS = 1e-16;

// Copies a block from the packed symmetric matrix to a dense square block.
static __inline__ void cpy_matrix_block_to_block(const double* a, int row, int column,
                                                 int matrix_size, int n, int m, double* b) {
  int i, j;

  memset(b, 0, n * m * sizeof(double));

  for (i = row; i < row + n; i++) {
    for (j = column; j < column + m; ++j) {
      b[(i - row) * m + (j - column)] = a[get_symmetric_index(i, j, matrix_size)];
    }
  }
}

// Copies a dense square block back into the packed symmetric matrix.
static __inline__ void cpy_block_to_matrix_block(double* a, int row, int column, int matrix_size,
                                                 int n, int m, const double* b) {
  int i, j;

  for (i = row; i < row + n; i++) {
    for (j = column; j < column + m; ++j) {
      a[get_symmetric_index(i, j, matrix_size)] = b[(i - row) * m + (j - column)];
    }
  }
}

// Performs block multiplication with diagonal scaling: C = C - A^T * D * B.
//
// Optimized with manual loop unrolling by 8 for high performance.
static inline void main_blocks_diagonal_multiply(int n, int m, int l, const double* a,
                                                 const double* b, const double* d, double* c) {
  int i, j, k;
  const double *pa, *pb;

  pa = a;
  pb = b;
  for (k = 0; k < n; ++k) {
    double pd = d[k];

    for (i = 0; i < m; ++i) {
      double ta = pa[i] * pd;
      double* pc = c + (size_t)i * l;

      for (j = 0; j < l - 7; j += 8) {
        pc[j] -= pb[j] * ta;
        pc[j + 1] -= pb[j + 1] * ta;
        pc[j + 2] -= pb[j + 2] * ta;
        pc[j + 3] -= pb[j + 3] * ta;
        pc[j + 4] -= pb[j + 4] * ta;
        pc[j + 5] -= pb[j + 5] * ta;
        pc[j + 6] -= pb[j + 6] * ta;
        pc[j + 7] -= pb[j + 7] * ta;
      }

      for (; j < l; ++j) {
        pc[j] -= pb[j] * ta;
      }
    }

    pa += m;
    pb += l;
  }
}

// Performs standard block multiplication: C = A * B.
//
// Optimized with manual loop unrolling by 8 for high performance.
static inline void main_blocks_multiply(int n, int m, int l, const double* a, const double* b,
                                        double* c) {
  int i, j, k;
  const double *pa, *pb;

  memset(c, 0, (size_t)m * l * sizeof(double));

  pa = a;
  pb = b;
  for (k = 0; k < n; ++k) {
    for (i = 0; i < m; ++i) {
      double ta = pa[i];
      double* pc = c + (size_t)i * l;

      for (j = 0; j < l - 7; j += 8) {
        pc[j] += pb[j] * ta;
        pc[j + 1] += pb[j + 1] * ta;
        pc[j + 2] += pb[j + 2] * ta;
        pc[j + 3] += pb[j + 3] * ta;
        pc[j + 4] += pb[j + 4] * ta;
        pc[j + 5] += pb[j + 5] * ta;
        pc[j + 6] += pb[j + 6] * ta;
        pc[j + 7] += pb[j + 7] * ta;
      }

      for (; j < l; ++j) {
        pc[j] += pb[j] * ta;
      }
    }

    pa += m;
    pb += l;
  }
}

static void cpy_diagonal_block_to_block(const double* a, int t, int matrix_size, int m, double* b) {
  int i, j;

  memset(b, 0, (size_t)m * m * sizeof(double));

  for (i = t; i < t + m; i++) {
    for (j = i; j < t + m; j++) {
      b[(i - t) * m + j - t] = a[get_symmetric_index(i, j, matrix_size)];
    }
  }
}

static void cpy_block_to_diagonal_block(double* a, int t, int matrix_size, int m, const double* b) {
  int i, j;

  for (i = t; i < t + m; i++) {
    for (j = i; j < t + m; j++) {
      a[get_symmetric_index(i, j, matrix_size)] = b[(i - t) * m + j - t];
    }
  }
}

// Inverts a triangular block with diagonal scaling.
static int inverse_upper_triangle_block_and_diagonal(int n, const double* a, const double* d,
                                                     double* b) {
  int i, j, k;
  double* pbi;

  memset(b, 0, (size_t)n * n * sizeof(double));
  for (i = 0; i < n; ++i) b[i * n + i] = d[i];

  pbi = b + (size_t)(n - 1) * n;
  for (i = n - 1; i >= 0; --i) {
    if (fabs(a[i * n + i]) < EPS) return -1;

    double dt = 1.0 / a[i * n + i];

    for (j = i; j < n; j++) pbi[j] *= dt;

    double* pbj = b;
    const double* pa = a;
    for (j = 0; j < i; ++j) {
      for (k = i; k < n; ++k) {
        pbj[k] -= pbi[k] * pa[i];
      }

      pbj += n;
      pa += n;
    }

    pbi -= n;
  }

  return 0;
}

// Performs standard Cholesky decomposition on a small dense block.
static int cholesky_for_block(int n, double* a, double* d) {
  int i, j, k;
  double* pai;

  for (i = 0; i < n; ++i) d[i] = 1.0;

  pai = a;
  for (i = 0; i < n; ++i) {
    double* pak = a;
    for (k = 0; k < i; ++k) {
      for (j = i; j < n; ++j) {
        pai[j] -= pak[i] * d[k] * pak[j];
      }

      pak += n;
    }

    if (pai[i] < 0.0) {
      d[i] = -1.0;
      pai[i] = -pai[i];
    }

    pai[i] = sqrt(pai[i]);

    if (fabs(pai[i]) < EPS) return -1;

    double dt = 1.0 / pai[i];
    for (j = i + 1; j < n - 7; j += 8) {
      pai[j] *= dt;
      pai[j + 1] *= dt;
      pai[j + 2] *= dt;
      pai[j + 3] *= dt;
      pai[j + 4] *= dt;
      pai[j + 5] *= dt;
      pai[j + 6] *= dt;
      pai[j + 7] *= dt;
    }

    for (; j < n; ++j) {
      pai[j] *= dt;
    }

    pai += n;
  }

  return 0;
}

static int inverse_upper_triangle_block_and_diagonal_rhs(int n, const double* a, const double* d,
                                                         double* rhs) {
  int i, j;

  for (i = 0; i < n; ++i) rhs[i] *= d[i];

  for (i = n - 1; i >= 0; --i) {
    if (fabs(a[i * n + i]) < EPS) return -1;

    rhs[i] *= 1.0 / a[i * n + i];

    for (j = 0; j < i; ++j) {
      rhs[j] -= rhs[i] * a[j * n + i];
    }
  }

  return 0;
}

static int inverse_lower_triangle_block_rhs(int n, const double* a, double* rhs) {
  int i, j;

  for (i = 0; i < n; ++i) {
    if (fabs(a[i * n + i]) < EPS) return -1;

    rhs[i] *= 1.0 / a[i * n + i];

    for (j = i + 1; j < n; ++j) {
      rhs[j] -= rhs[i] * a[i * n + j];
    }
  }

  return 0;
}

static void matrix_block_vector_multiply(int n, int m, const double* a, const double* b,
                                         double* c) {
  int i, j;
  const double* pai;

  pai = a;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      c[i] -= pai[j] * b[j];
    }

    pai += m;
  }
}

static void matrix_block_transposed_vector_multiply(int n, int m, const double* a, const double* b,
                                                    double* c) {
  int i, j;

  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      c[i] -= a[j * m + i] * b[j];
    }
  }
}

int cholesky(CholeskyMatrix* matrix, double* workspace) {
  int i, j, k;
  int matrix_size = matrix->size;
  int block_size = matrix->block_size;
  double* matrix_data = matrix->data;
  double* diagonal = matrix->diagonal;

  double *ma, *mb, *mc;
  ma = workspace;
  mb = ma + (size_t)block_size * block_size;
  mc = mb + (size_t)block_size * block_size;

  for (i = 0; i < matrix_size; i += block_size) {
    for (j = i; j < matrix_size; j += block_size) {
      int pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
      int pij_m = (j + block_size < matrix_size ? block_size : matrix_size - j);

      if (j != i)
        cpy_matrix_block_to_block(matrix_data, i, j, matrix_size, pij_n, pij_m, mc);
      else
        cpy_diagonal_block_to_block(matrix_data, i, matrix_size, pij_n, mc);

      for (k = 0; k < i; k += block_size) {
        int pki_n = (k + block_size < matrix_size ? block_size : matrix_size - k);
        int pki_m = (i + block_size < matrix_size ? block_size : matrix_size - i);

        int pkj_n = (k + block_size < matrix_size ? block_size : matrix_size - k);
        int pkj_m = (j + block_size < matrix_size ? block_size : matrix_size - j);

        cpy_matrix_block_to_block(matrix_data, k, i, matrix_size, pki_n, pki_m, ma);
        cpy_matrix_block_to_block(matrix_data, k, j, matrix_size, pkj_n, pkj_m, mb);

        main_blocks_diagonal_multiply(pki_n, pki_m, pkj_m, ma, mb, diagonal + k, mc);
      }

      if (j != i)
        cpy_block_to_matrix_block(matrix_data, i, j, matrix_size, pij_n, pij_m, mc);
      else
        cpy_block_to_diagonal_block(matrix_data, i, matrix_size, pij_n, mc);
    }

    int pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);

    cpy_diagonal_block_to_block(matrix_data, i, matrix_size, pij_n, mb);

    if (cholesky_for_block(pij_n, mb, diagonal + i)) return -1;

    cpy_block_to_diagonal_block(matrix_data, i, matrix_size, pij_n, mb);

    if (inverse_upper_triangle_block_and_diagonal(pij_n, mb, diagonal + i, ma)) return -1;

    for (j = i + block_size; j < matrix_size; j += block_size) {
      int cur_pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
      int cur_pij_m = (j + block_size < matrix_size ? block_size : matrix_size - j);

      cpy_matrix_block_to_block(matrix_data, i, j, matrix_size, cur_pij_n, cur_pij_m, mb);
      main_blocks_multiply(cur_pij_n, cur_pij_n, cur_pij_m, ma, mb, mc);
      cpy_block_to_matrix_block(matrix_data, i, j, matrix_size, cur_pij_n, cur_pij_m, mc);
    }
  }

  return 0;
}

int solve_lower_triangle_matrix_system(const CholeskyMatrix* matrix, double* rhs,
                                       double* workspace) {
  int i, j;
  int matrix_size = matrix->size;
  int block_size = matrix->block_size;
  const double* matrix_data = matrix->data;
  double* ma;
  ma = workspace;

  for (i = 0; i < matrix_size; i += block_size) {
    int pii_n = (i + block_size < matrix_size ? block_size : matrix_size - i);

    cpy_diagonal_block_to_block(matrix_data, i, matrix_size, pii_n, ma);

    if (inverse_lower_triangle_block_rhs(pii_n, ma, rhs + i)) return -1;

    for (j = i + block_size; j < matrix_size; j += block_size) {
      int pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
      int pij_m = (j + block_size < matrix_size ? block_size : matrix_size - j);

      cpy_matrix_block_to_block(matrix_data, i, j, matrix_size, pij_n, pij_m, ma);

      matrix_block_transposed_vector_multiply(pij_n, pij_m, ma, rhs + i, rhs + j);
    }
  }

  return 0;
}

int solve_upper_triangle_matrix_diagonal_system(const CholeskyMatrix* matrix, double* rhs,
                                                double* workspace) {
  int i, j;
  int matrix_size = matrix->size;
  int block_size = matrix->block_size;
  const double* matrix_data = matrix->data;
  const double* diagonal = matrix->diagonal;
  int residue;
  double* ma;

  ma = workspace;

  residue = matrix_size - (matrix_size % block_size);
  if (residue == matrix_size) residue -= block_size;

  for (i = residue; i >= 0; i -= block_size) {
    for (j = residue; j > i; j -= block_size) {
      int pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
      int pij_m = (j + block_size < matrix_size ? block_size : matrix_size - j);

      cpy_matrix_block_to_block(matrix_data, i, j, matrix_size, pij_n, pij_m, ma);

      matrix_block_vector_multiply(pij_n, pij_m, ma, rhs + j, rhs + i);
    }

    int pii_n = (i + block_size < matrix_size ? block_size : matrix_size - i);

    cpy_diagonal_block_to_block(matrix_data, i, matrix_size, pii_n, ma);

    if (inverse_upper_triangle_block_and_diagonal_rhs(pii_n, ma, diagonal + i, rhs + i)) return -1;
  }

  return 0;
}
