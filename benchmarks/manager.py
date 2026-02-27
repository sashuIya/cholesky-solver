#!/usr/bin/env python3
import subprocess
import re
import os
import sys
from datetime import datetime

# Configuration
MATRIX_SIZES = [1000, 2000, 3000, 4000, 5000]
BLOCK_SIZES = [64, 128]
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE = os.path.join(PROJECT_ROOT, "build", "cholesky_solver")
RESULTS_DIR = os.path.join(PROJECT_ROOT, "benchmarks", "results")

def build_project():
    print("Building project...")
    try:
        subprocess.check_call(["make", "-C", os.path.join(PROJECT_ROOT, "src")], 
                             stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print(f"Error: Build failed with exit code {e.returncode}")
        sys.exit(1)

def run_single_benchmark(n, m):
    try:
        result = subprocess.run([EXE, str(n), str(m)], capture_output=True, text=True, check=True)
        output = result.stdout
        
        # Parse time: Time: total=00:00:06.80, on algorithm=00:00:00.06
        time_match = re.search(r'Time: total=(\d{2}):(\d{2}):(\d{2})\.(\d{2}),\s+on algorithm', output)
        # Parse residual: Residual: 8.37906e-15
        res_match = re.search(r'Residual:\s+([eE\d\.-]+)', output)
        
        if time_match and res_match:
            h, mins, s, c = map(int, time_match.groups())
            total_seconds = h * 3600 + mins * 60 + s + c / 100.0
            return total_seconds, res_match.group(1)
    except Exception as e:
        print(f"Error running benchmark N={n}, M={m}: {e}")
    return None, None

def get_latest_baseline():
    if not os.path.exists(RESULTS_DIR):
        return None
    files = [os.path.join(RESULTS_DIR, f) for f in os.listdir(RESULTS_DIR) if f.endswith('.txt')]
    if not files:
        return None
    return max(files, key=os.path.getmtime)

def parse_result_file(filename):
    results = {}
    with open(filename, 'r') as f:
        for line in f:
            # Try new format first: 5000     | 128    |      6.17s | 1.52649e-06
            match = re.search(r'(\d+)\s+\|\s+(\d+)\s+\|\s+([\d\.]+)s\s+\|\s+([eE\d\.-]+)', line)
            if match:
                n, m = match.group(1), match.group(2)
                results[(n, m)] = (float(match.group(3)), match.group(4))
                continue
            
            # Try old format: 5000         | 128          | 00:00:07.06     | 1.52649e-06
            match = re.search(r'(\d+)\s+\|\s+(\d+)\s+\|\s+(\d{2}):(\d{2}):(\d{2})\.(\d{2})\s+\|\s+([eE\d\.-]+)', line)
            if match:
                n, m = match.group(1), match.group(2)
                h, mins, s, c = map(int, match.groups()[2:6])
                total_seconds = h * 3600 + mins * 60 + s + c / 100.0
                results[(n, m)] = (total_seconds, match.group(7))
    return results

def run_benchmarks(save=False):
    build_project()
    print(f"{'Matrix':<8} | {'Block':<6} | {'Time':<10} | {'Residual'}")
    print("-" * 45)
    
    current_results = {}
    lines = []
    for n in MATRIX_SIZES:
        for m in BLOCK_SIZES:
            if m <= n:
                t, res = run_single_benchmark(n, m)
                if t is not None:
                    line = f"{n:<8} | {m:<6} | {t:>9.2f}s | {res}"
                    print(line)
                    lines.append(line)
                    current_results[(str(n), str(m))] = (t, res)

    if save:
        os.makedirs(RESULTS_DIR, exist_ok=True)
        try:
            commit = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"], 
                                           text=True).strip()
            filename = os.path.join(RESULTS_DIR, f"{commit}.txt")
        except:
            filename = os.path.join(RESULTS_DIR, f"bench_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt")
        
        with open(filename, 'w') as f:
            f.write("\n".join(lines) + "\n")
        print(f"\nResults saved to {filename}")
    
    return current_results

def compare_results(baseline_file, current_data, threshold=10.0):
    baseline_data = parse_result_file(baseline_file)
    print(f"\nComparing against baseline: {os.path.basename(baseline_file)}")
    print(f"{'Matrix':<8} | {'Block':<6} | {'Baseline':<9} | {'Current':<9} | {'Diff %':<8} | {'Status'}")
    print("-" * 65)
    
    has_regression = False
    for key in sorted(baseline_data.keys(), key=lambda x: (int(x[0]), int(x[1]))):
        if key in current_data:
            b_time, _ = baseline_data[key]
            c_time, _ = current_data[key]
            
            diff_pct = ((c_time - b_time) / b_time * 100.0) if b_time > 0 else 0
            
            status = "OK"
            if diff_pct > threshold:
                status = "REGRESSION"
                has_regression = True
            elif diff_pct < -threshold:
                status = "IMPROVEMENT"
            
            n, m = key
            print(f"{n:<8} | {m:<6} | {b_time:>8.2f}s | {c_time:>8.2f}s | {diff_pct:>+7.1f}% | {status}")
    
    if has_regression:
        print("\nWARNING: Performance regression detected!")
        return False
    return True

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Cholesky Solver Benchmark Manager")
    parser.add_argument("command", choices=["run", "check", "save"], help="Command to run")
    parser.add_argument("--threshold", type=float, default=10.0, help="Regression threshold in %%")
    args = parser.parse_args()

    if args.command == "run":
        run_benchmarks(save=False)
    elif args.command == "save":
        run_benchmarks(save=True)
    elif args.command == "check":
        latest = get_latest_baseline()
        if not latest:
            print("No baseline found. Running first benchmark to establish one...")
            run_benchmarks(save=True)
        else:
            current = run_benchmarks(save=False)
            if not compare_results(latest, current, args.threshold):
                sys.exit(1)
