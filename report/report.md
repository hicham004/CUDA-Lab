# 3D Cellular Automata Simulation Using CUDA

**Student:** [Your Name]  
**Course:** [Course Name]  
**Lab:** 3D Cellular Automata Simulation Using CUDA  

## Introduction

A cellular automaton (CA) is a discrete computational model in which a grid of cells evolves over time according to local update rules. In this lab, the automaton is defined on a 3D grid of size `N x N x N`, where each cell is either alive (`1`) or dead (`0`). The goal of the lab is to implement both a sequential CPU version and a CUDA GPU version of the same 3D cellular automaton, verify that they produce exactly identical final states, and measure the performance improvement obtained with GPU parallelism.

The 3D update rule uses the full 26-neighbor neighborhood around each cell. A dead cell becomes alive if it has exactly 5 alive neighbors. An alive cell dies if it has fewer than 4 or more than 6 alive neighbors. Otherwise, the state remains unchanged. Boundary cells are kept fixed as dead.

## Implementation Details

### Shared Infrastructure

The project uses shared host-side utilities to keep the sequential and CUDA implementations consistent. The initial grid is generated deterministically on the host from a fixed seed and alive probability, which guarantees that both implementations begin from the exact same starting state. The grid is stored as a 1D array using the linear indexing formula:

`idx = i * N * N + j * N + k`

This layout is used in both the CPU and GPU paths.

### Sequential CPU Implementation

The sequential version uses two buffers, `current` and `next`, to avoid overwriting values needed for the same time step. For each interior cell, the implementation counts all 26 neighbors and applies the lab rule exactly. Boundary cells are never updated and therefore remain dead throughout the simulation.

### CUDA GPU Implementation

The CUDA version maps one cell to one thread using 3D thread/block indexing. The initial state is generated on the host and copied to device memory. Two device buffers are used for time stepping, just like the sequential version. Each kernel launch updates one full time step, and the buffers are swapped after each step. Boundary cells are explicitly forced to zero in the kernel to match the sequential fixed-dead boundary policy exactly.

### Verification

Correctness is verified by comparing the final CUDA grid against the final sequential grid cell by cell. The verification performed in the experiments below passed for every tested CUDA configuration.

### Timing Methodology

Sequential timing measures only the CPU update loop. CUDA timing measures only the repeated kernel execution region, with `cudaDeviceSynchronize()` used before starting and after finishing timing. Initialization, verification, and file output are excluded from the reported times. This follows a consistent methodology across all runs.

## Experiments and Results

### Experimental Setup

- Environment: Google Colab
- GPU: Tesla T4
- CUDA compiler: 12.8
- CMake: 3.31.10
- Host compiler: g++ 11.4.0
- Time steps: `T = 50`
- Seed: `12345`
- Alive probability: `0.3`

### Experiment Matrix

The following grid sizes were tested:

- `N = 64`
- `N = 128`
- `N = 256`

The following CUDA block sizes were tested:

- `8x8x8`
- `16x8x8`
- `8x16x8`

### Timing Summary

| Grid Size | Sequential (ms) | CUDA 8x8x8 (ms) | Speedup | CUDA 16x8x8 (ms) | Speedup | CUDA 8x16x8 (ms) | Speedup |
|---|---:|---:|---:|---:|---:|---:|---:|
| 64 | 728.352 | 16.146 | 45.11 | 15.050 | 48.39 | 12.142 | 59.98 |
| 128 | 6455.980 | 174.552 | 36.99 | 136.042 | 47.46 | 127.484 | 50.64 |
| 256 | 51636.900 | 791.870 | 65.21 | 804.915 | 64.15 | 819.561 | 63.01 |

### Verification Summary

All CUDA runs passed exact final-state verification with `0` mismatches.

### Best CUDA Configuration Per Grid Size

- N=64: best block size was `8x16x8` with `12.142 ms` and speedup `59.98x`.
- N=128: best block size was `8x16x8` with `127.484 ms` and speedup `50.64x`.
- N=256: best block size was `8x8x8` with `791.870 ms` and speedup `65.21x`.

### Plots

- `report/assets/plots/execution_time_vs_grid_size.png`
- `report/assets/plots/speedup_vs_grid_size.png`

## Analysis

The results show a large and consistent performance advantage for the CUDA implementation over the sequential CPU implementation. The sequential runtime increases sharply with grid size, as expected for a 3D stencil-style update over `N^3` cells and 50 time steps. The CUDA implementation also slows down as the grid size increases, but the growth is much smaller in absolute terms than the CPU version.

For `N = 64`, the best performing block size was `8x16x8`, with a speedup of about `59.98x`. For `N = 128`, the same block size remained the best, reaching about `50.64x`. For `N = 256`, `8x8x8` became slightly faster than the other tested block sizes, reaching about `65.21x`. This suggests that the effect of block shape is real, but it depends on the workload size and the underlying GPU scheduling behavior.

Another important result is correctness. Every CUDA run matched the sequential reference exactly, which confirms that the CUDA kernel uses the same initialization, same boundary policy, same 26-neighbor rule, and same time-stepping behavior as the CPU implementation.

## Conclusion

This lab successfully implemented a 3D cellular automaton in both sequential C++ and CUDA. The CUDA version was verified against the sequential reference and produced exactly identical final states for all tested configurations. Performance results showed substantial speedups, ranging from about `36.99x` to `65.21x` depending on grid size and block configuration. Overall, the CUDA version demonstrates the value of GPU parallelism for large 3D grid simulations while preserving exact correctness.

## Submission Notes

Before final submission, replace the placeholder student/course information at the top of this report.