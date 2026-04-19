# Project Plan

## Source of Truth

The attached lab instructions require:

1. A sequential CPU implementation of the 3D cellular automaton.
2. A CUDA GPU implementation of the same automaton.
3. Exact final-state verification between sequential and CUDA runs.
4. Timing measurements for multiple grid sizes, with speedup calculations.
5. At least three CUDA block-size experiments.
6. A report with implementation details, experiments, plots, tables, analysis, and conclusion.

## What To Implement

### Core simulation

- 3D grid of size `N x N x N`.
- Cell states are `0` for dead and `1` for alive.
- Each cell uses the full 26-neighbor 3D neighborhood.
- Update rules:
  - Dead cell becomes alive with exactly 5 alive neighbors.
  - Alive cell dies with fewer than 4 or more than 6 alive neighbors.
  - Otherwise the state stays unchanged.
- Use double buffering: `current_grid` and `next_grid`.
- Run for `T` time steps, with `T = 50` as the baseline from the instructions.

### Sequential version

- Implement the update loop on the CPU.
- Measure execution time for at least `N = 64`, `128`, and `256`.

### CUDA version

- Map one cell to one CUDA thread.
- Use 3D block and thread indexing.
- Copy the seeded initial state from host to device.
- Run the kernel for each time step.
- Copy the final state back to host.
- Measure CUDA execution time correctly.

### Verification

- Sequential and CUDA runs must start from the exact same initial state.
- Sequential and CUDA final grids must match exactly, cell by cell.

### Performance work

- Record execution times.
- Compute `speedup = T_seq / T_cuda`.
- Test at least three block shapes such as:
  - `8x8x8`
  - `16x8x8`
  - `8x16x8`

## Proposed Project Structure

```text
CUDA Lab/
|-- CMakeLists.txt
|-- README.md
|-- docs/
|   `-- project-plan.md
|-- include/
|   `-- ca/
|       `-- config.hpp
|-- src/
|   |-- shared/
|   |   `-- config.cpp
|   |-- sequential/
|   |   `-- main.cpp
|   `-- cuda/
|       `-- main.cu
|-- scripts/
|   |-- common.ps1
|   |-- build.ps1
|   |-- run_seq.ps1
|   |-- run_cuda.ps1
|   `-- run_experiments.ps1
|-- results/
|   |-- raw/
|   |   |-- timings/
|   |   |   `-- README.md
|   |   `-- final_states/
|   |       `-- README.md
|   `-- processed/
|       `-- tables/
|           `-- README.md
`-- report/
    `-- assets/
        |-- plots/
        |   `-- README.md
        `-- tables/
            `-- README.md
```

## Exact Build And Run Commands

### Preferred wrapper scripts

Build:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Config Release
```

Run the sequential executable:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_seq.ps1 -Config Release -Size 64 -Steps 50 -Seed 12345 -AliveProbability 0.3
```

Run the CUDA executable with one block shape:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_cuda.ps1 -Config Release -Size 64 -Steps 50 -Seed 12345 -AliveProbability 0.3 -BlockX 8 -BlockY 8 -BlockZ 8
```

Print the planned experiment commands:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_experiments.ps1 -Config Release
```

Execute the experiment plan:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_experiments.ps1 -Config Release -Execute
```

### Raw CMake commands

Configure:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCUDA_LAB_ENABLE_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=native
```

Build:

```powershell
cmake --build build --config Release
```

Run the sequential binary directly:

```powershell
.\build\bin\Release\ca_seq.exe --size 64 --steps 50 --seed 12345 --alive-prob 0.3
```

Run the CUDA binary directly:

```powershell
.\build\bin\Release\ca_cuda.exe --size 64 --steps 50 --seed 12345 --alive-prob 0.3 --block-x 8 --block-y 8 --block-z 8
```

## Risk Checklist

### Exact reproducibility

- Do not generate the initial grid independently in the CPU and GPU paths.
- Generate the initial state once on the host from a fixed seed, then reuse that exact buffer for both versions.
- Keep the alive probability exactly `0.3` and record the seed in output metadata.

### Exact sequential/CUDA final-state matching

- Use the same rules, same boundary policy, same indexing formula, and same number of time steps.
- Use double buffering in both versions so updates are based only on the previous step.
- Verify full-grid equality, not just counts or hashes.

### Boundary handling

- The instructions say edge cells can remain fixed as dead.
- That policy must be identical in CPU and CUDA.
- Avoid accidental wraparound or partially updated edges.
- Guard all CUDA accesses carefully to avoid out-of-bounds reads.

### Timing methodology

- CPU timing should cover only the actual update loop, not setup or file I/O unless explicitly labeled.
- CUDA timing should clearly state whether it includes host-device transfers or kernel-only time.
- If timing kernels, synchronize before stopping the timer.
- Keep the methodology consistent across all runs and document it in the report.

### CUDA block-size experiments

- Compare the same workload across all block sizes.
- Keep `N`, `T`, seed, and initialization identical while changing only block shape.
- Be careful with invalid or oversized block shapes for the target GPU.
- Record the tested block shape with every timing result.

## Current Scaffold Status

Implemented now:

- Folder layout
- CMake build scaffold
- Shared command-line configuration parsing
- Sequential placeholder executable
- CUDA placeholder executable
- PowerShell wrapper scripts
- Output and report asset directories

Not implemented yet:

- CPU automaton update loop
- CUDA kernel and device memory flow
- Final-state serialization
- Verification logic
- Real timing output
- Experiment data collection
- Report plots and tables

## Next Implementation Order

1. Implement shared grid representation, seeded initialization, and final-state file I/O.
2. Implement the sequential CPU automaton and verify the boundary policy.
3. Implement the CUDA kernel and device-side stepping with the same logic.
4. Add exact host-side comparison between CPU and CUDA final states.
5. Add timing output and experiment result serialization.
6. Run the required grid-size and block-size experiments.
7. Generate report tables and plots.
