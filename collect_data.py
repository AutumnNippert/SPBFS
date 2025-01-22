#!/usr/bin/env python3
import sys
import os
import json
import subprocess
import pandas as pd

def parse_args(argv):
    """
    Parses and validates command-line arguments.

    Usage:
        collect_raw_data_flexible.py <algorithm> <problem>
                                     <thread_range> <thread_jump>
                                     <extra_expansion_range> <extra_expansion_jump>
                                     <data_path> <repetitions>

    Example:
        ./collect_raw_data_flexible.py astar path 2-4 1 0-5 1 ./samples 3
    """
    if len(argv) < 9:
        print("Usage: {} <algorithm> <problem> "
              "<thread_range> <thread_jump> "
              "<extra_expansion_range> <extra_expansion_jump> "
              "<data_path> <repetitions>".format(argv[0]))
        sys.exit(1)

    algorithm = argv[1]
    problem = argv[2]

    # thread_range e.g. "2-4"
    thread_range_str = argv[3]
    thread_jump = int(argv[4])

    # extra_expansion_range e.g. "0-5"
    extra_range_str = argv[5]
    extra_jump = int(argv[6])

    data_path = argv[7]
    repetitions = int(argv[8])

    # Parse thread_range_str
    try:
        t_start_str, t_end_str = thread_range_str.split('-')
        t_start = int(t_start_str)
        t_end = int(t_end_str)
    except ValueError:
        print("[ERROR] thread_range must be 'start-end' (e.g. '2-4').")
        sys.exit(1)
    threads = list(range(t_start, t_end + 1, thread_jump))

    # Parse extra_expansion_range
    try:
        e_start_str, e_end_str = extra_range_str.split('-')
        e_start = int(e_start_str)
        e_end = int(e_end_str)
    except ValueError:
        print("[ERROR] extra_expansion_range must be 'start-end' (e.g. '0-5').")
        sys.exit(1)
    extras = list(range(e_start, e_end + 1, extra_jump))

    # Handle data_path
    if os.path.isdir(data_path):
        data_files = [
            os.path.join(data_path, f)
            for f in os.listdir(data_path)
            if os.path.isfile(os.path.join(data_path, f))
        ]
        if not data_files:
            print(f"[ERROR] No files found in directory '{data_path}'.")
            sys.exit(1)
    elif os.path.isfile(data_path):
        data_files = [data_path]
    else:
        print(f"[ERROR] '{data_path}' is not a valid file or directory.")
        sys.exit(1)

    return {
        "algorithm": algorithm,
        "problem": problem,
        "threads": threads,
        "extras": extras,
        "data_files": data_files,
        "repetitions": repetitions
    }

def run_one_case(algorithm, problem, extra, thread, file_path):
    """
    Runs `./main` once with the specified parameters.
    Returns (ok, data_dict) where:
        ok = True/False (did it succeed?)
        data_dict = dict with JSON fields if succeeded, otherwise {}
    """
    print(f"[INFO] Running: {algorithm} {problem} {extra} {thread} {file_path}")
    cmd = [
        "./main",
        "-a", algorithm,
        "-p", problem,
        "-e", str(extra),
        "-t", str(thread)
    ]
    try:
        with open(file_path, "r") as infile:
            result = subprocess.run(
                cmd,
                stdin=infile,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
    except Exception as exc:
        print(f"[ERROR] Could not run: {cmd}\nException: {exc}")
        return False, {}

    if result.returncode != 0:
        print(f"[WARNING] Command failed (returncode={result.returncode}): {cmd}")
        print("stderr:\n", result.stderr)
        return False, {}

    # Parse JSON
    try:
        data = json.loads(result.stdout)
        return True, data
    except json.JSONDecodeError:
        print("[WARNING] JSON decode error. Output was:")
        print(result.stdout)
        return False, {}

def append_to_csv_flexible(df_new, csv_path):
    """
    Appends df_new to csv_path, allowing for new or missing columns.

    If csv_path does not exist, writes df_new as a new file with header.
    If csv_path exists, read it in, unify columns with df_new, and overwrite.

    This ensures that if new runs contain new fields/columns, they will be added
    to the entire CSV, and old rows will have NaN for those new columns.
    """
    if os.path.exists(csv_path):
        # Read existing CSV
        df_old = pd.read_csv(csv_path)

        # Build superset of columns
        all_cols = sorted(set(df_old.columns).union(set(df_new.columns)))
        
        # Reindex each to the superset of columns (missing -> NaN)
        df_old = df_old.reindex(columns=all_cols)
        df_new = df_new.reindex(columns=all_cols)

        # Concatenate
        df_out = pd.concat([df_old, df_new], ignore_index=True)

        # Write out (overwrite) with a header
        df_out.to_csv(csv_path, index=False)
        print(f"[INFO] Updated '{csv_path}' with {len(df_new)} new rows. Total rows = {len(df_out)}.")
    else:
        # Write a new file with header
        df_new.to_csv(csv_path, index=False)
        print(f"[INFO] Created '{csv_path}' with {len(df_new)} rows.")

def main():
    # 1. Parse arguments
    args = parse_args(sys.argv)
    algorithm = args["algorithm"]
    problem = args["problem"]
    threads = args["threads"]
    extras = args["extras"]
    data_files = args["data_files"]
    repetitions = args["repetitions"]

    # 2. Prepare a list to accumulate raw data
    raw_records = []

    try:
        # 3. Nested loops over extras, threads, data_files, repetitions
        for e_val in extras:
            for t_val in threads:
                for file_path in data_files:
                    for rep_i in range(repetitions):
                        ok, json_data = run_one_case(
                            algorithm, problem, e_val, t_val, file_path
                        )
                        if not ok:
                            # We'll just skip if the run failed
                            continue

                        # We'll build a single row of data.
                        row = {
                            "algorithm": algorithm,
                            "problem": problem,
                            "threads": t_val,
                            "extra_expansion": e_val,
                            "input_file": os.path.basename(file_path),
                            "repetition": rep_i + 1
                        }

                        # Copy the JSON keys/values from ./main output 
                        for key, val in json_data.items():
                            row[key] = val

                        raw_records.append(row)
    except KeyboardInterrupt:
        print("\n[INFO] Interrupted by user.")
    finally:
        if not raw_records:
            print("[INFO] No successful runs to record.")
            return

        # 4. Convert to DataFrame
        df_new_runs = pd.DataFrame(raw_records)

        # 5. Append to (or create) CSV, ensuring new columns are handled
        output_dir = "raw_output"
        os.makedirs(output_dir, exist_ok=True)
        csv_path = os.path.join(output_dir, "raw_data_flexible.csv")

        append_to_csv_flexible(df_new_runs, csv_path)



if __name__ == "__main__":
    main()
