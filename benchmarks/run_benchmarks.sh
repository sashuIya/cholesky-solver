#!/bin/bash

# Benchmark script for Cholesky Solver

PROJECT_ROOT=$(pwd)
SRC_DIR="$PROJECT_ROOT/src"
BUILD_DIR="$PROJECT_ROOT/build"
EXE="$BUILD_DIR/cholesky_solver"

# Ensure the project is built
echo "Building project..."
cd "$SRC_DIR" || exit
make clean && make
cd "$PROJECT_ROOT" || exit

if [ ! -f "$EXE" ]; then
    echo "Error: Executable not found at $EXE"
    exit 1
fi

echo "Running benchmarks..."
echo "--------------------------------------------------------------------------------"
printf "%-12s | %-12s | %-15s | %-12s
" "Matrix Size" "Block Size" "Time (Total)" "Residual"
echo "--------------------------------------------------------------------------------"

# Matrix sizes and block sizes to test
MATRIX_SIZES=(1000 2000 3000 4000 5000)
BLOCK_SIZES=(64 128)

for N in "${MATRIX_SIZES[@]}"; do
    for M in "${BLOCK_SIZES[@]}"; do
        if [ "$M" -le "$N" ]; then
            # Run the solver and capture output
            # We use a temp file to capture output and parse it
            OUTPUT=$("$EXE" "$N" "$M" 2>&1)
            
            # Extract total time and residual using grep/awk
            # Sample line: Time: total=00:00:00.05, on algorithm=00:00:00.05
            TOTAL_TIME=$(echo "$OUTPUT" | grep "on algorithm" | awk -F'total=' '{print $2}' | awk -F',' '{print $1}')
            
            # Sample line: Error: 5.46075e-15 ; Residual: 8.37906e-15 (1.20155e-16)
            RESIDUAL=$(echo "$OUTPUT" | grep "Residual:" | awk -F'Residual: ' '{print $2}' | awk '{print $1}')
            
            printf "%-12s | %-12s | %-15s | %-12s
" "$N" "$M" "$TOTAL_TIME" "$RESIDUAL"
        fi
    done
done

echo "--------------------------------------------------------------------------------"
echo "Benchmarks completed."
