#!/bin/bash

# Robustness tests for Cholesky Solver

EXE="./build/cholesky_solver"

# Ensure the project is built
make -C src

echo "Running robustness tests..."

# Test 1: Invalid arguments (non-numeric)
echo -n "Test 1 (Invalid arguments): "
$EXE abc 32 2>/dev/null | grep -q "Error: invalid matrix size 'abc'"
if [ $? -eq 0 ]; then echo "PASS"; else echo "FAIL"; fi

# Test 2: Invalid block size (larger than matrix)
echo -n "Test 2 (Block size > Matrix size): "
$EXE 10 20 2>/dev/null | grep -q "Error: invalid block size '20'"
if [ $? -eq 0 ]; then echo "PASS"; else echo "FAIL"; fi

# Test 3: Non-existent input file
echo -n "Test 3 (Non-existent file): "
$EXE 10 2 non_existent.txt 2>/dev/null | grep -q "Error: cannot open input file"
if [ $? -eq 0 ]; then echo "PASS"; else echo "FAIL"; fi

# Test 4: Malformed input file (insufficient data)
echo "1.0 2.0" > malformed.txt
echo -n "Test 4 (Malformed file): "
$EXE 3 1 malformed.txt 2>/dev/null | grep -q "Error: failed to read element"
if [ $? -eq 0 ]; then echo "PASS"; else echo "FAIL"; fi

# Test 5: Extra data in input file (warning)
echo "1 2 3 4 5 6 7 8 9 10" > extra_data.txt # 3x3 symmetric matrix needs 6 unique elements
echo -n "Test 5 (Extra data warning): "
$EXE 3 1 extra_data.txt 2>&1 | grep -q "Warning: extra data found"
if [ $? -eq 0 ]; then echo "PASS"; else echo "FAIL"; fi

# Cleanup
rm malformed.txt extra_data.txt

echo "Robustness tests completed."
