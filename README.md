# Cholesky Solver

## Overview
This program is a solver for symmetric linear systems ($Ax = b$). It is based on the **Block Cholesky Decomposition** method and is optimized for performance on large-scale systems.

### Performance Features
1.  **Block Matrix Layout:** The matrix is stored and processed in blocks ($m \times m$) to maximize CPU cache utilization.
2.  **Manual Loop Unrolling:** Hot loops in the matrix multiplication and decomposition phases are manually unrolled by a factor of 8.

> **Key Audit Finding (2026):** Empirical benchmarking confirmed that manual loop unrolling is critical for this implementation. Attempting to rely solely on modern compiler optimizations (GCC -O3) resulted in a ~40-50% performance degradation on large matrices ($N=5000$). The manual unrolling has been preserved and standardized.

### Recent Refactorings
-   **Standardized Style:** Codebase updated to Google C Style.
-   **Improved Memory Management:** Switched from a single large memory block to individual allocations with centralized cleanup for better safety and leak prevention.
-   **Centralized Indexing:** Symmetric matrix indexing is now handled by a dedicated utility function, reducing logic complexity across the project.
-   **Static Analysis:** Code has been audited with `cppcheck` and `clang-tidy` to ensure type safety and robustness.

## Usage

### Building
The project uses a standard Makefile. From the root directory:
```bash
make -C src
```
This will create the executable in the `build/` directory.

### Running
```bash
./build/cholesky_solver (matrix_size) (block_size) [matrix_input_file]
```
- `matrix_size`: Dimension of the symmetric matrix.
- `block_size`: Size of the square blocks used in the algorithm.
- `matrix_input_file` (Optional): Path to a file containing the matrix elements. If omitted, the program generates a test matrix.

### Benchmarking
A benchmarking script is provided to evaluate performance across different scales:
```bash
./benchmarks/run_benchmarks.sh
```

## Documentation
Mathematical details and algorithm descriptions can be found in the `doc/` directory (requires LaTeX to build).

## License
Copyright 2011-2012 Alexander Lapin.
Distributed under the GNU General Public License v3.0. See `LICENSE` for details.

## Contacts
Alexander Lapin, <lapinra@gmail.com>
