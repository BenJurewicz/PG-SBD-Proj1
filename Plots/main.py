import os
import re
import matplotlib.pyplot as plt


def parse_data_file(filepath):
    """Parses a data file to extract metrics."""
    with open(filepath, "r") as f:
        content = f.read()

    def get_value(pattern):
        match = re.search(pattern, content)
        return float(match.group(1)) if match else None

    record_count_match = re.search(r"results-(\d+)-", os.path.basename(filepath))
    record_count = int(record_count_match.group(1)) if record_count_match else 0

    return {
        "record_count": record_count,
        "phases_actual": get_value(r"Phases Needed Actual: (\d+\.?\d*)"),
        "phases_theory": get_value(r"Phases Needed Theory: (\d+\.?\d*)"),
        "disk_accesses_practice": get_value(r"Disk accesses in practice: (\d+\.?\d*)"),
        "disk_accesses_theory": get_value(r"Disk accesses in theory:(\d+\.?\d*)"),
    }


def plot_comparisons(data):
    """Generates and saves comparison plots."""
    # Sort data by record count for correct plotting
    data.sort(key=lambda x: x["record_count"])

    record_counts = [d["record_count"] for d in data]
    phases_actual = [d["phases_actual"] for d in data]
    phases_theory = [d["phases_theory"] for d in data]
    disk_accesses_practice = [d["disk_accesses_practice"] for d in data]
    disk_accesses_theory = [d["disk_accesses_theory"] for d in data]

    # Plot 1: Phases Comparison
    plt.figure(figsize=(10, 6))
    plt.plot(record_counts, phases_actual, marker="o", label="Actual Phases")
    plt.plot(record_counts, phases_theory, marker="x", label="Theoretical Phases")
    plt.title("Actual vs. Theoretical Phases")
    plt.xlabel("Record Count")
    plt.ylabel("Phases")
    plt.xscale("log")
    plt.legend()
    plt.grid(True)
    plt.savefig("phases_comparison.png")
    plt.close()

    # Plot 2: Disk Accesses Comparison
    plt.figure(figsize=(10, 6))
    plt.plot(
        record_counts,
        disk_accesses_practice,
        marker="o",
        label="Disk Accesses in Practice",
    )
    plt.plot(
        record_counts, disk_accesses_theory, marker="x", label="Disk Accesses in Theory"
    )
    plt.title("Practice vs. Theory Disk Accesses")
    plt.xlabel("Record Count")
    plt.ylabel("Disk Accesses")
    plt.xscale("log")
    plt.yscale("log")
    plt.legend()
    plt.grid(True)
    plt.savefig("disk_accesses_comparison.png")
    plt.close()

    print("Plots generated: 'phases_comparison.png', 'disk_accesses_comparison.png'")


if __name__ == "__main__":
    data_dir = "data"
    files = [
        os.path.join(data_dir, f)
        for f in os.listdir(data_dir)
        if f.startswith("results-") and f.endswith(".txt")
    ]

    if not files:
        print("No data files found in the 'data' directory.")
    else:
        all_data = [parse_data_file(f) for f in files]
        plot_comparisons(all_data)

