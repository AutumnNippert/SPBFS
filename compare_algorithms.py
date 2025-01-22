#!/usr/bin/env python3

from testing_utils import parse_range, gather_data_files, create_output_dir, plot_for_expansion

import sys
def parse_arguments():
    """
    Parses command-line arguments (via sys.argv).
    Expected usage:

        compare_algorithms_modular.py <thread_range> <thread_jump> <expansions>
                                      <data_path>
                                      <alg1> <alg2> ... <algN>

    Returns a dictionary of:
        {
          "thread_range_str": str,
          "thread_jump": int,
          "expansions": int,
          "data_path": str,
          "algorithms": [str, ...]
        }
    Exits if arguments are invalid or missing.
    """
    if len(sys.argv) < 6:  # Updated to reflect the new argument count
        print("Usage: {} <thread_range> <thread_jump> "
              "<data_path> <alg1> [<alg2> ... <algN>]".format(sys.argv[0]))
        sys.exit(1)

    thread_range_str = sys.argv[1]
    try:
        thread_jump = int(sys.argv[2])
    except ValueError:
        print("[ERROR] thread_jump must be an integer.")
        sys.exit(1)

    
    try:
        expansions = int(sys.argv[3])
    except ValueError:
        print("[ERROR] expansions must be an integer.", sys.argv[3])
        sys.exit(1)

    data_path = sys.argv[4]  # Updated index for data_path
    if len(sys.argv) < 5:  # Updated to reflect the new argument count
        print("[ERROR] Please provide at least one algorithm name.")
        sys.exit(1)

    algorithms = sys.argv[5:]  # Updated index for algorithms

    return {
        "thread_range_str": thread_range_str,
        "thread_jump": thread_jump,
        "expansions": expansions,
        "data_path": data_path,
        "algorithms": algorithms
    }

def main():
    args = parse_arguments()
    threads = parse_range(args["thread_range_str"], args["thread_jump"])
    expansions = args["expansions"]
    output_dir = create_output_dir("compare_output")

    plot_for_expansion(
        exp_val=expansions,
        threads=threads,
        algorithms=args["algorithms"],
        output_dir=output_dir
    )

if __name__ == "__main__":
    main()
