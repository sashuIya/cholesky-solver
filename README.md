<p align="center">
  <img src="assets/logo.png" alt="Cholesky Solver Logo" width="400">
</p>

# Cholesky Solver

## Overview
This program is a high-performance solver for symmetric linear systems ($Ax = b$). It is based on the **Block Cholesky Decomposition** method and is intentionally designed as a **single-threaded** implementation to maximize serial efficiency and cache utilization.

### Performance Features
1.  **Block Matrix Layout:** The matrix is stored and processed in blocks ($m \times m$) to maximize CPU cache utilization.
2.  **Manual Loop Unrolling:** Hot loops in the matrix multiplication and decomposition phases are manually unrolled by a factor of 8.
3.  **Single-Threaded Design:** Optimized for sequential execution, avoiding synchronization overhead.

> **Key Audit Finding (2026):** Empirical benchmarking confirmed that manual loop unrolling is critical for this implementation. Attempting to rely solely on modern compiler optimizations (GCC -O3) resulted in a ~40-50% performance degradation on large matrices ($N=5000$). The manual unrolling has been preserved and standardized.

## Optimization Techniques

To achieve high performance in a single-threaded environment, several specialized techniques are employed:

### 1. Packed Symmetric Storage
To reduce memory footprint by 50%, the solver only stores the upper triangular part of the symmetric matrix in a packed format. This improves spatial locality and reduces cache misses during large-scale computations.

### 2. Cache-Friendly Memory Access
The block-wise approach ensures that the "working set" of data fits within the L1/L2 CPU caches. By processing the matrix in $m \times m$ blocks, the algorithm minimizes the movement of data between main memory and the processor.

### 3. Loop Unrolling (Factor of 8)
Critical nested loops in the block multiplication kernels are manually unrolled. This technique:
- Reduces the number of branch instructions.
- Increases the number of independent operations available for the CPU's instruction-level parallelism (ILP).
- Helps the compiler generate more efficient SIMD instructions.

### 4. Specialized BLAS-like Kernels
The solver uses dedicated internal kernels for operations like $C = C - A^T D B$. These kernels are tailored for the specific data layout of the packed symmetric matrix, avoiding the overhead of general-purpose linear algebra libraries.

## Mathematical Foundation

The solver uses the $A = R^T D R$ decomposition, where:
- $A$ is a symmetric matrix.
- $R$ is an upper triangular matrix with positive diagonal elements.
- $D$ is a diagonal matrix with elements $\pm 1$.

### Block Algorithm
The matrix $A$ is partitioned into blocks $A_{ij}$. The blocks of $R$ and $D$ are calculated as follows:

1.  **Diagonal Block:**
    $R_{ii}^T D_i R_{ii} = A_{ii} - \sum_{k=1}^{i-1} R_{ki}^T D_k R_{ki}$
    $R_{ii}$ and $D_i$ are found by applying the standard Cholesky method to the resulting block.

2.  **Off-Diagonal Blocks:**
    $R_{ij} = D_i^{-1} (R_{ii}^T)^{-1} \left( A_{ij} - \sum_{k=1}^{i-1} R_{ki}^T D_k R_{kj} \right)$

### Solving the System
Once $A = R^T D R$ is computed, the system $Ax = b$ is solved in two steps:
1.  Solve $R^T y = b$ for $y$ (Forward substitution).
2.  Solve $D R x = y$ for $x$ (Backward substitution).

## Recent Refactorings
-   **Standardized Style:** Codebase updated to Google C Style with Google-style docstrings.
-   **Architectural Split:** Core logic extracted into a reusable `Solver Engine` library.
-   **Improved Memory Management:** Switched to individual allocations with centralized cleanup.
-   **Centralized Indexing:** Symmetric matrix indexing is now handled by a dedicated utility function.
-   **Robustness:** Added `strtol` for argument parsing and enhanced file I/O error checking.
-   **Automation:** Integrated a Python-based benchmark manager (`benchmarks/manager.py`).

## Usage

### Building
The project uses a standard Makefile. From the root directory:
```bash
make -C src
```

### Running
```bash
./build/cholesky_solver (matrix_size) (block_size) [matrix_input_file]
```
- `matrix_size`: Dimension of the symmetric matrix.
- `block_size`: Size of the square blocks.
- `matrix_input_file` (Optional): Path to a file containing matrix elements.

### Benchmarking
```bash
./benchmarks/manager.py run   # Run once
./benchmarks/manager.py check # Compare against latest baseline
```

## License
Copyright 2011-2012 Alexander Lapin.
Distributed under the GNU General Public License v3.0.

## Contacts
Alexander Lapin, <lapinra@gmail.com>
