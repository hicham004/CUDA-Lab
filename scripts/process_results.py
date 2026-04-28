#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import re
from pathlib import Path


BLOCK_ORDER = {"8x8x8": 0, "16x8x8": 1, "8x16x8": 2}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Parse raw timing logs into processed experiment CSV files."
    )
    parser.add_argument(
        "--input-dir",
        default="results/raw/timings",
        help="Directory containing raw .txt timing outputs.",
    )
    parser.add_argument(
        "--output-dir",
        default="results/processed/tables",
        help="Directory where processed CSV files will be written.",
    )
    return parser.parse_args()


def parse_row(path: Path) -> dict[str, object]:
    text = path.read_text(encoding="utf-8")

    block_match = re.search(r"_B([^.]+)\.txt$", path.name)
    impl = "cuda" if block_match else "sequential"
    block = block_match.group(1) if block_match else ""

    size_match = re.search(r"^\s*size\s*=\s*([0-9]+)\s*$", text, re.MULTILINE)
    steps_match = re.search(r"^\s*steps\s*=\s*([0-9]+)\s*$", text, re.MULTILINE)
    seed_match = re.search(r"^\s*seed\s*=\s*([0-9]+)\s*$", text, re.MULTILINE)
    alive_match = re.search(r"^\s*alive_prob\s*=\s*([0-9.]+)\s*$", text, re.MULTILINE)
    live_cells = re.findall(r"^\s*live_cells\s*=\s*([0-9]+)\s*$", text, re.MULTILINE)
    checksums = re.findall(r"^\s*checksum\s*=\s*(0x[0-9a-fA-F]+)\s*$", text, re.MULTILINE)
    elapsed_match = re.search(r"^\s*elapsed_ms\s*=\s*([0-9.]+)\s*$", text, re.MULTILINE)

    if not size_match or not steps_match or not seed_match or not alive_match or len(live_cells) < 2 or len(checksums) < 2 or not elapsed_match:
        raise ValueError(f"Could not parse required values from '{path.name}'.")

    size = int(size_match.group(1))
    steps = int(steps_match.group(1))
    seed = int(seed_match.group(1))
    alive_probability = float(alive_match.group(1))

    verification = ""
    if re.search(r"status\s*=\s*PASS", text):
        verification = "PASS"
    elif re.search(r"status\s*=\s*FAIL", text):
        verification = "FAIL"

    return {
        "impl": impl,
        "size": size,
        "block": block,
        "steps": steps,
        "seed": seed,
        "alive_probability": alive_probability,
        "initial_live_cells": int(live_cells[0]),
        "final_live_cells": int(live_cells[1]),
        "initial_checksum": checksums[0],
        "final_checksum": checksums[1],
        "elapsed_ms": float(elapsed_match.group(1)),
        "verification": verification,
    }


def sort_key(row: dict[str, object]) -> tuple[int, int, int]:
    impl_rank = 0 if row["impl"] == "sequential" else 1
    block_rank = BLOCK_ORDER.get(str(row["block"]), 99)
    return (int(row["size"]), impl_rank, block_rank)


def main() -> int:
    args = parse_args()
    input_dir = Path(args.input_dir)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    rows = [parse_row(path) for path in sorted(input_dir.glob("*.txt"))]
    if not rows:
        raise FileNotFoundError(f"No .txt timing files found in '{input_dir}'.")

    seq_times = {
        int(row["size"]): float(row["elapsed_ms"])
        for row in rows
        if row["impl"] == "sequential"
    }

    for row in rows:
        if row["impl"] == "sequential":
            row["speedup_vs_seq"] = 1.0
        else:
            row["speedup_vs_seq"] = seq_times[int(row["size"])] / float(row["elapsed_ms"])

    rows.sort(key=sort_key)

    experiment_path = output_dir / "experiment_results.csv"
    fieldnames = [
        "impl",
        "size",
        "block",
        "steps",
        "seed",
        "alive_probability",
        "initial_live_cells",
        "final_live_cells",
        "initial_checksum",
        "final_checksum",
        "elapsed_ms",
        "verification",
        "speedup_vs_seq",
    ]
    with experiment_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    best_rows = []
    for size in sorted(seq_times):
        cuda_rows = [row for row in rows if row["impl"] == "cuda" and row["size"] == size]
        best_rows.append(min(cuda_rows, key=lambda row: float(row["elapsed_ms"])))

    best_path = output_dir / "best_cuda_by_size.csv"
    best_fieldnames = [
        "size",
        "block",
        "elapsed_ms",
        "speedup_vs_seq",
        "verification",
        "final_checksum",
    ]
    with best_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=best_fieldnames)
        writer.writeheader()
        for row in best_rows:
            writer.writerow({key: row[key] for key in best_fieldnames})

    print(f"Wrote {experiment_path}")
    print(f"Wrote {best_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
