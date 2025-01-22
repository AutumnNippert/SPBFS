#!/usr/bin/env python3
import sys
import os
import subprocess
import json
import pandas as pd
import matplotlib.pyplot as plt

def parse_arguments():
    """
    Parses command-line arguments (via sys.argv).
    Expected usage:

        compare_algorithms_modular.py <thread_range> <thread_jump>
                                      <expansion_range> <expansion_jump>
                                      <data_path>
                                      <alg1> <alg2> ... <algN>

    Returns a dictionary of:
        {
          "thread_range_str": str,
          "thread_jump": int,
          "expansion_range_str": str,
          "expansion_jump": int,
          "data_path": str,
          "algorithms": [str, ...]
        }
    Exits if arguments are invalid or missing.
    """
    if len(sys.argv) < 7:
        print("Usage: {} <thread_range> <thread_jump> "
              "<expansion_range> <expansion_jump> <repeats> <data_path> <alg1> [<alg2> ... <algN>]".format(sys.argv[0]))
        sys.exit(1)

    thread_range_str = sys.argv[1]
    try:
        thread_jump = int(sys.argv[2])
    except ValueError:
        print("[ERROR] thread_jump must be an integer.")
        sys.exit(1)

    expansion_range_str = sys.argv[3]
    try:
        expansion_jump = int(sys.argv[4])
    except ValueError:
        print("[ERROR] expansion_jump must be an integer.")
        sys.exit(1)

    try:
        repeats = int(sys.argv[5])
    except ValueError:
        print("[ERROR] repeats must be an integer.")
        sys.exit(1)

    data_path = sys.argv[6]
    if len(sys.argv) < 8:
        print("[ERROR] Please provide at least one algorithm name.")
        sys.exit(1)

    algorithms = sys.argv[7:]  # all remaining args are algorithms

    return {
        "thread_range_str": thread_range_str,
        "thread_jump": thread_jump,
        "expansion_range_str": expansion_range_str,
        "expansion_jump": expansion_jump,
        "repeats": repeats,
        "data_path": data_path,
        "algorithms": algorithms
    }

def parse_range(range_str, jump):
    """
    Given a range string like '2-4' and an integer jump, returns a list of values.
    Example:
      parse_range('2-4', 1) -> [2, 3, 4]
      parse_range('0-6', 2) -> [0, 2, 4, 6]
    Exits if the format is invalid.
    """
    try:
        start_str, end_str = range_str.split('-')
        start_val = int(start_str)
        end_val = int(end_str)
    except ValueError:
        print(f"[ERROR] range must be 'start-end' (e.g. '2-4'), got '{range_str}'")
        sys.exit(1)

    return list(range(start_val, end_val + 1, jump))

def gather_data_files(data_path):
    """
    If data_path is a directory, returns a list of all files in it.
    If data_path is a file, returns a single-element list with that file.
    Exits if invalid.
    """
    if os.path.isdir(data_path):
        files = [
            os.path.join(data_path, f)
            for f in os.listdir(data_path)
            if os.path.isfile(os.path.join(data_path, f))
        ]
        if not files:
            print(f"[ERROR] No files in directory '{data_path}'.")
            sys.exit(1)
        return files

    elif os.path.isfile(data_path):
        return [data_path]

    else:
        print(f"[ERROR] '{data_path}' is not a valid file or directory.")
        sys.exit(1)

def get_avg_elapsed(algorithm, thread, expansion):
    """
    Runs ./main for the given (algorithm, thread, expansion) across all data_files.
    Returns the average Elapsed Time. If none succeeded, returns 0.
    """
    df = pd.read_csv("raw_output/raw_data_flexible.csv")

    # check all columns exist
    expected = ['Algorithm', 'Threads', 'Extra Expansion Time', 'Elapsed Time']
    if not all(col in df.columns for col in expected):
        print("[ERROR] Missing columns in raw_data_flexible.csv")
        print("[ERROR] Expected columns:", expected)
        print("[ERROR] Found columns:", df.columns)
        sys.exit(1) 

    df = df[(df['Algorithm'] == algorithm) & (df['Threads'] == thread) & (df['Extra Expansion Time'] == expansion)]
    if df.empty:
        return 0
    
    return df['Elapsed Time'].mean()


def create_output_dir(dirname="compare_output"):
    """
    Creates the output directory if it doesn't exist.
    Returns the directory path (string).
    """
    os.makedirs(dirname, exist_ok=True)
    return dirname

def plot_for_expansion(exp_val, threads, algorithms, output_dir):
    """
    Creates a plot of avg Elapsed Time vs. threads for all given algorithms, 
    at a fixed expansion value. Saves the figure to:
       output_dir/elapsed_time_exp_<exp_val>.png
    """
    plt.figure(figsize=(8, 5))

    all_y_vals = []
    for alg in algorithms:
        y_vals = []
        for t_val in threads:
            avg_t = get_avg_elapsed(alg, t_val, exp_val)
            y_vals.append(avg_t)
            all_y_vals.append(avg_t)

        plt.plot(threads, y_vals, marker='o', linestyle='-', label=alg)

    plt.title(f"Elapsed Time vs. Threads (Exp={exp_val})")
    plt.xlabel("Threads")
    plt.ylabel("Avg Elapsed Time (s)")
    plt.grid(True)
    plt.legend()

    # Auto-scale Y-axis
    if all_y_vals:
        min_y = min(all_y_vals)
        max_y = max(all_y_vals)
        y_lower = 0 if min_y >= 0 else min_y * 1.1
        y_upper = max_y * 1.1 if max_y > 0 else 1.0
        if abs(y_upper - y_lower) < 1e-9:
            y_lower -= 1
            y_upper += 1
        plt.ylim([y_lower, y_upper])

    out_file = os.path.join(output_dir, f"elapsed_time_exp_{exp_val}.png")
    plt.savefig(out_file)
    plt.close()
    print(f"[INFO] Saved plot: {out_file}")