#!/usr/bin/env python3

from testing_utils import parse_range, create_output_dir
import pandas as pd
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import cm

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
    algorithms = args["algorithms"]

    # Load the CSV file
    df = pd.read_csv("raw_output/raw_data_flexible.csv")

    # Filter expansions
    df = df[df["Extra Expansion Time"] == expansions]
    #filter by algorithms inputted
    df = df[df["Algorithm"].isin(algorithms)]

    # Ensure columns are present
    required_columns = ["Algorithm", "Elapsed Time", "Threads"]
    for column in required_columns:
        if column not in df.columns:
            raise ValueError(f"Missing required column: {column}")

    # Ensure A* goes through all thread counts
    all_threads = pd.DataFrame({"Threads": threads})
    a_star_data = df[df["Algorithm"] == "A*"]
    if not a_star_data.empty:
        a_star_data = a_star_data.groupby("Threads")["Elapsed Time"].mean().reset_index()
        a_star_data = pd.merge(all_threads, a_star_data, on="Threads", how="left").fillna(method="ffill").fillna(method="bfill")
        a_star_data["Algorithm"] = "A*"
        df = pd.concat([df[df["Algorithm"] != "A*"], a_star_data], ignore_index=True)

    # Group by Algorithm and Threads, then calculate the average Elapsed Time
    grouped_data = (
        df.groupby(["Algorithm", "Threads"])["Elapsed Time"]
        .mean()
        .reset_index()
    )

    # Pivot data to make it suitable for plotting
    pivoted_data = grouped_data.pivot(index="Threads", columns="Algorithm", values="Elapsed Time")

    # Plot the data
    plt.figure(figsize=(10, 6))
    for algorithm in pivoted_data.columns:
        plt.plot(pivoted_data.index, pivoted_data[algorithm], label=algorithm, marker="o")

    # Add labels, title, and legend
    plt.xlabel("Threads", fontsize=12)
    plt.ylabel("Average of Elapsed Time", fontsize=12)
    plt.title("Average of Elapsed Time by Threads and Algorithm", fontsize=14)
    plt.legend(title="Algorithm", fontsize=10)
    plt.grid(True, linestyle="--", alpha=0.7)

    # Save the plot to a file
    output_file = f"{output_dir}/average_elapsed_time_by_threads.png"
    plt.tight_layout()
    plt.savefig(output_file)
    print(f"Plot saved to {output_file}")
    plt.show()

def plot_3d_elapsed_time(df, output_dir, algorithm, filename):
    # Ensure necessary columns are present
    required_columns = ["Threads", "Extra Expansion Time", "Elapsed Time", "Algorithm"]
    for column in required_columns:
        if column not in df.columns:
            raise ValueError(f"Missing required column: {column}")

    # Filter the data for the specified algorithm
    df = df[df["Algorithm"] == algorithm]
    #if filename is a number
    if filename.isnumeric():
        df = df[df["input_file"] == int(filename)]
    else:
        df = df[df["input_file"] == filename]


    # Group by Threads and Extra Expansion Time, then calculate the average Elapsed Time
    grouped_data = (
        df.groupby(["Threads", "Extra Expansion Time"])["Elapsed Time"]
        .mean()
        .reset_index()
    )

    # Extract data for 3D plotting
    X = grouped_data["Threads"]
    Y = grouped_data["Extra Expansion Time"]
    Z = grouped_data["Elapsed Time"]

    # Create a 3D plot
    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection="3d")
    
    # Plot the surface
    scatter = ax.scatter(X, Y, Z, c=Z, cmap="viridis", marker="o")
    
    # Add labels and title
    ax.set_xlabel("Threads", fontsize=12)
    ax.set_ylabel("Extra Expansion Time", fontsize=12)
    ax.set_zlabel("Average Elapsed Time", fontsize=12)
    ax.set_title(f"3D Visualization of Average Elapsed Time for {algorithm}", fontsize=14)
    
    # Add color bar
    cbar = fig.colorbar(scatter, ax=ax, shrink=0.6, aspect=10)
    cbar.set_label("Elapsed Time", fontsize=12)
    
    # Save the 3D plot to a file
    output_file = f"{output_dir}/3d_elapsed_time_plot_{algorithm}.png"
    plt.tight_layout()
    plt.savefig(output_file)
    print(f"3D plot saved to {output_file}")

def plot_3d_elapsed_time_plane(df, output_dir, algorithm, filename):
    # Ensure necessary columns are present
    required_columns = ["Threads", "Extra Expansion Time", "Elapsed Time", "Algorithm"]
    for column in required_columns:
        if column not in df.columns:
            raise ValueError(f"Missing required column: {column}")

    # Filter the data for the specified algorithm
    df = df[df["Algorithm"] == algorithm]
    if filename.isnumeric():
        df = df[df["input_file"] == int(filename)]
    else:
        df = df[df["input_file"] == filename]

    # Group by Threads and Extra Expansion Time, then calculate the average Elapsed Time
    grouped_data = (
        df.groupby(["Threads", "Extra Expansion Time"])["Elapsed Time"]
        .mean()
        .reset_index()
    )

    # Extract data for 3D plotting
    X = grouped_data["Threads"].values
    Y = grouped_data["Extra Expansion Time"].values
    Z = grouped_data["Elapsed Time"].values

    # Create a meshgrid for the surface plot
    X_unique = grouped_data["Threads"].unique()
    Y_unique = grouped_data["Extra Expansion Time"].unique()
    X_mesh, Y_mesh = np.meshgrid(X_unique, Y_unique)

    # Reshape Z to match the meshgrid
    Z_mesh = grouped_data.pivot(index="Extra Expansion Time", columns="Threads", values="Elapsed Time").values

    # Create a 3D plot
    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection="3d")

    # Plot the surface
    surface = ax.plot_surface(X_mesh, Y_mesh, Z_mesh, cmap=cm.viridis, edgecolor="none", alpha=0.8)

    # Add labels and title
    ax.set_xlabel("Threads", fontsize=12)
    ax.set_ylabel("Extra Expansion Time", fontsize=12)
    ax.set_zlabel("Average Elapsed Time", fontsize=12)
    ax.set_title(f"3D Surface Plot of Average Elapsed Time for {algorithm}", fontsize=14)

    # Add color bar
    cbar = fig.colorbar(surface, ax=ax, shrink=0.6, aspect=10)
    cbar.set_label("Elapsed Time", fontsize=12)

    # Save the 3D plot to a file
    output_file = f"{output_dir}/3d_surface_elapsed_time_plane_plot_{algorithm}.png"
    plt.tight_layout()
    plt.savefig(output_file)
    print(f"3D surface plot saved to {output_file}")



if __name__ == "__main__":
    main()
    df = pd.read_csv("raw_output/raw_data_flexible.csv")
    # plot_3d_elapsed_time(df, ".", "CAFE", "3")
    # plot_3d_elapsed_time(df, ".", "KBFS", "3")
    # plot_3d_elapsed_time(df, ".", "A*", "3")
    # plot_3d_elapsed_time(df, ".", "SPA*", "3")
    # plot_3d_elapsed_time_plane(df, ".", "CAFE", "3")
    # plot_3d_elapsed_time_plane(df, ".", "KBFS", "3")
    # plot_3d_elapsed_time_plane(df, ".", "A*", "3")
    # plot_3d_elapsed_time_plane(df, ".", "SPA*", "3")