#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
from pathlib import Path

try:
    import matplotlib.pyplot as plt
except ImportError as exc:  # pragma: no cover - import guard for user environments
    raise SystemExit(
        "matplotlib is required to regenerate plots. Install it with 'pip install matplotlib'."
    ) from exc


BLOCK_ORDER = ["8x8x8", "16x8x8", "8x16x8"]
BLOCK_STYLES = {
    "8x8x8": {"marker": "o", "linestyle": "-", "color": "#d62728"},
    "16x8x8": {"marker": "s", "linestyle": "--", "color": "#ff7f0e"},
    "8x16x8": {"marker": "^", "linestyle": "-.", "color": "#2ca02c"},
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate experiment plots from a processed CSV file."
    )
    parser.add_argument(
        "--csv",
        default="results/processed/tables/experiment_results.csv",
        help="Processed experiment CSV file.",
    )
    parser.add_argument(
        "--output-dir",
        default="report/assets/plots",
        help="Directory where PNG plots will be written.",
    )
    return parser.parse_args()


def load_rows(path: Path) -> list[dict[str, object]]:
    with path.open("r", newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        rows: list[dict[str, object]] = []
        for row in reader:
            row["size"] = int(row["size"])
            row["elapsed_ms"] = float(row["elapsed_ms"])
            row["speedup_vs_seq"] = float(row["speedup_vs_seq"])
            rows.append(row)
        return rows


def main() -> int:
    args = parse_args()
    csv_path = Path(args.csv)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    rows = load_rows(csv_path)
    seq_rows = [row for row in rows if row["impl"] == "sequential"]
    cuda_rows = [row for row in rows if row["impl"] == "cuda"]

    plt.figure(figsize=(8, 5))
    plt.plot(
        [row["size"] for row in seq_rows],
        [row["elapsed_ms"] for row in seq_rows],
        marker="D",
        linestyle="-",
        color="#1f77b4",
        linewidth=2,
        label="Sequential",
    )
    for block in BLOCK_ORDER:
        block_rows = [row for row in cuda_rows if row["block"] == block]
        style = BLOCK_STYLES[block]
        plt.plot(
            [row["size"] for row in block_rows],
            [row["elapsed_ms"] for row in block_rows],
            marker=style["marker"],
            linestyle=style["linestyle"],
            color=style["color"],
            linewidth=2,
            label=f"CUDA {block}",
        )
    plt.xlabel("Grid Size N")
    plt.ylabel("Execution Time (ms, log scale)")
    plt.title("Execution Time vs Grid Size")
    plt.yscale("log")
    plt.grid(True, which="both", linestyle=":", alpha=0.7)
    plt.legend()
    plt.tight_layout()
    execution_plot = output_dir / "execution_time_vs_grid_size.png"
    plt.savefig(execution_plot, dpi=200)
    plt.close()

    plt.figure(figsize=(8, 5))
    for block in BLOCK_ORDER:
        block_rows = [row for row in cuda_rows if row["block"] == block]
        style = BLOCK_STYLES[block]
        plt.plot(
            [row["size"] for row in block_rows],
            [row["speedup_vs_seq"] for row in block_rows],
            marker=style["marker"],
            linestyle=style["linestyle"],
            color=style["color"],
            linewidth=2,
            label=block,
        )
    plt.xlabel("Grid Size N")
    plt.ylabel("Speedup (T_seq / T_cuda)")
    plt.title("Speedup vs Grid Size")
    plt.grid(True, linestyle=":", alpha=0.7)
    plt.legend()
    plt.tight_layout()
    speedup_plot = output_dir / "speedup_vs_grid_size.png"
    plt.savefig(speedup_plot, dpi=200)
    plt.close()

    print(f"Wrote {execution_plot}")
    print(f"Wrote {speedup_plot}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
