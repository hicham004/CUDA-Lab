# CUDA Lab

This repository contains a 3D cellular automaton implemented in sequential C++ and CUDA. The project includes deterministic host-side initialization, exact sequential/CUDA final-state verification, raw timing logs from a Google Colab Tesla T4 run, processed result tables, and final plots used in the Lab 3 submission.

## Repository Structure

- `include/ca/` shared headers for configuration, rules, grid storage, summaries, and verification
- `src/shared/` host-side initialization, summaries, and comparison logic
- `src/sequential/` sequential CPU implementation
- `src/cuda/` CUDA kernel and device-side orchestration
- `scripts/` build, experiment, result-processing, and plotting utilities
- `results/raw/timings/` captured console outputs from experiment runs
- `results/processed/tables/` processed CSV summaries derived from the raw logs
- `report/assets/plots/` final execution-time and speedup plots

## Experiment Baseline

The verified experiment set used:

- grid sizes `64`, `128`, `256`
- CUDA block sizes `8x8x8`, `16x8x8`, `8x16x8`
- `50` time steps
- seed `12345`
- alive probability `0.3`

All CUDA runs matched the sequential reference exactly with `0` mismatches.

## Best CUDA Results

| Grid Size | Best Block | CUDA Time (ms) | Speedup |
|---|---|---:|---:|
| 64 | `8x16x8` | 12.142 | 59.98x |
| 128 | `8x16x8` | 127.484 | 50.64x |
| 256 | `8x8x8` | 791.870 | 65.21x |

## Included Artifacts

- raw timing logs in `results/raw/timings/`
- processed result tables in `results/processed/tables/`
- final plots in `report/assets/plots/`
