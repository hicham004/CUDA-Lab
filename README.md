# CUDA Lab

3D cellular automata simulation implemented in sequential C++ and CUDA, with exact final-state verification and experiment artifacts generated from a real Google Colab T4 run.

## Project Layout

- `include/ca/`: shared headers for configuration, grid storage, rules, summaries, CUDA/sequential interfaces, and verification.
- `src/shared/`: deterministic host-side initialization, grid helpers, summaries, and comparison logic.
- `src/sequential/`: sequential CPU implementation and CLI entry point.
- `src/cuda/`: CUDA kernel, device orchestration, and CLI entry point.
- `scripts/`: PowerShell helpers plus result-processing and plotting utilities.
- `results/raw/timings/`: raw captured outputs from experiment runs.
- `results/processed/tables/`: processed CSV summaries.
- `report/assets/plots/`: generated plots used in the report.
- `report/report.md`: submission-ready report draft.

## Build

### Windows PowerShell

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Config Release
```

If your CUDA machine needs an explicit architecture:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Config Release -CudaArchitectures native
```

### Colab / Linux

```bash
cmake -S . -B build-colab -DCMAKE_BUILD_TYPE=Release -DCUDA_LAB_ENABLE_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=75
cmake --build build-colab --config Release -j
```

## Run Sequential

### Windows PowerShell

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_seq.ps1 -Config Release -Size 64 -Steps 50 -Seed 12345 -AliveProbability 0.3
```

### Colab / Linux

```bash
./build-colab/bin/Release/ca_seq --size 64 --steps 50 --seed 12345 --alive-prob 0.3
```

## Run CUDA

### Windows PowerShell

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_cuda.ps1 -Config Release -Size 64 -Steps 50 -Seed 12345 -AliveProbability 0.3 -BlockX 8 -BlockY 8 -BlockZ 8
```

### Colab / Linux

```bash
./build-colab/bin/Release/ca_cuda --size 64 --steps 50 --seed 12345 --alive-prob 0.3 --block-x 8 --block-y 8 --block-z 8
```

## Run Exact Verification

### Windows PowerShell

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_cuda.ps1 -Config Release -Size 64 -Steps 50 -Seed 12345 -AliveProbability 0.3 -BlockX 8 -BlockY 8 -BlockZ 8 -Verify
```

### Colab / Linux

```bash
./build-colab/bin/Release/ca_cuda --size 64 --steps 50 --seed 12345 --alive-prob 0.3 --block-x 8 --block-y 8 --block-z 8 --verify
```

## Run The Experiment Suite

### Windows PowerShell

Dry run:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_experiments.ps1 -Config Release
```

Execute:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_experiments.ps1 -Config Release -Execute
```

By default, the experiment launcher enables CUDA verification and writes:

- raw console logs to `results/raw/timings/*.txt`
- metrics JSON to `results/raw/timings/*.json`
- optional final-state binaries to `results/raw/final_states/*.bin`

The default experiment matrix is:

- Grid sizes: `64`, `128`, `256`
- Block sizes: `8x8x8`, `16x8x8`, `8x16x8`
- Steps: `50`
- Seed: `12345`
- Alive probability: `0.3`

## Regenerate Processed Tables And Plots

Generate CSV summaries from raw `.txt` timing outputs:

```powershell
python .\scripts\process_results.py --input-dir .\results\raw\timings --output-dir .\results\processed\tables
```

Generate the two report plots from `experiment_results.csv`:

```powershell
python .\scripts\plot_results.py --csv .\results\processed\tables\experiment_results.csv --output-dir .\report\assets\plots
```

The execution-time plot uses a logarithmic y-axis so the sequential and CUDA curves remain readable on the same figure.

## Verified Baseline

These results were executed on Google Colab with a Tesla T4 GPU and exact verification enabled for every CUDA run.

| Grid Size | Best Block | CUDA Time (ms) | Speedup |
|---|---|---:|---:|
| 64 | `8x16x8` | 12.142 | 59.98x |
| 128 | `8x16x8` | 127.484 | 50.64x |
| 256 | `8x8x8` | 791.870 | 65.21x |

All CUDA runs matched the sequential reference exactly with `0` mismatches.

## Report And Artifacts

- Report draft: [report/report.md](report/report.md)
- Raw timing logs: [results/raw/timings](results/raw/timings)
- Processed tables: [results/processed/tables](results/processed/tables)
- Plots: [report/assets/plots](report/assets/plots)
