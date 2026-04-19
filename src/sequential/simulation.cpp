#include "ca/sequential.hpp"

#include "ca/host_init.hpp"
#include "ca/rules.hpp"

#include <array>
#include <chrono>
#include <ostream>
#include <utility>

namespace ca {
namespace {

void step_sequential(const Grid3D& current, Grid3D& next) {
  const int size = current.size();

  for (int i = 1; i < size - 1; ++i) {
    for (int j = 1; j < size - 1; ++j) {
      for (int k = 1; k < size - 1; ++k) {
        int live_neighbors = 0;

        for (int di = -1; di <= 1; ++di) {
          for (int dj = -1; dj <= 1; ++dj) {
            for (int dk = -1; dk <= 1; ++dk) {
              if (di == 0 && dj == 0 && dk == 0) {
                continue;
              }

              live_neighbors += current.at(i + di, j + dj, k + dk) != 0 ? 1 : 0;
            }
          }
        }

        next.at(i, j, k) = apply_rule(current.at(i, j, k), live_neighbors);
      }
    }
  }
}

bool boundaries_are_dead(const Grid3D& grid) {
  const int size = grid.size();

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      for (int k = 0; k < size; ++k) {
        if (Grid3D::is_boundary(size, i, j, k) && grid.at(i, j, k) != 0) {
          return false;
        }
      }
    }
  }

  return true;
}

bool run_single_test(const char* name, bool passed, std::ostream& output) {
  output << (passed ? "[PASS] " : "[FAIL] ") << name << '\n';
  return passed;
}

bool test_deterministic_boundary_initialization(std::ostream& output) {
  Grid3D first_grid(4);
  Grid3D second_grid(4);
  initialize_grid_host(first_grid, 7U, 1.0);
  initialize_grid_host(second_grid, 7U, 1.0);

  bool passed = boundaries_are_dead(first_grid) && boundaries_are_dead(second_grid);
  for (int i = 1; i < first_grid.size() - 1 && passed; ++i) {
    for (int j = 1; j < first_grid.size() - 1 && passed; ++j) {
      for (int k = 1; k < first_grid.size() - 1; ++k) {
        if (first_grid.at(i, j, k) != 1 || second_grid.at(i, j, k) != first_grid.at(i, j, k)) {
          passed = false;
          break;
        }
      }
    }
  }

  return run_single_test("deterministic initialization keeps boundaries dead", passed, output);
}

bool test_birth_rule(std::ostream& output) {
  Grid3D current(5);
  Grid3D next(5);

  current.fill(0);
  next.fill(0);

  const std::array<std::array<int, 3>, 5> neighbors = {{
      {{1, 2, 2}},
      {{2, 1, 2}},
      {{2, 2, 1}},
      {{2, 3, 2}},
      {{3, 2, 2}},
  }};

  for (const auto& neighbor : neighbors) {
    current.at(neighbor[0], neighbor[1], neighbor[2]) = 1;
  }

  step_sequential(current, next);

  const bool passed = next.at(2, 2, 2) == 1 && boundaries_are_dead(next);
  return run_single_test("dead cell with exactly five neighbors becomes alive", passed, output);
}

bool test_underpopulation_rule(std::ostream& output) {
  Grid3D current(5);
  Grid3D next(5);

  current.fill(0);
  next.fill(0);

  current.at(2, 2, 2) = 1;
  current.at(1, 2, 2) = 1;
  current.at(2, 1, 2) = 1;
  current.at(2, 2, 1) = 1;

  step_sequential(current, next);

  const bool passed = next.at(2, 2, 2) == 0;
  return run_single_test("alive cell with fewer than four neighbors dies", passed, output);
}

bool test_survival_rule(std::ostream& output) {
  Grid3D current(5);
  Grid3D next(5);

  current.fill(0);
  next.fill(0);

  current.at(2, 2, 2) = 1;
  current.at(1, 2, 2) = 1;
  current.at(2, 1, 2) = 1;
  current.at(2, 2, 1) = 1;
  current.at(2, 3, 2) = 1;

  step_sequential(current, next);

  const bool passed = next.at(2, 2, 2) == 1;
  return run_single_test("alive cell with four neighbors survives", passed, output);
}

bool test_overpopulation_rule(std::ostream& output) {
  Grid3D current(5);
  Grid3D next(5);

  current.fill(0);
  next.fill(0);

  current.at(2, 2, 2) = 1;

  const std::array<std::array<int, 3>, 7> neighbors = {{
      {{1, 2, 2}},
      {{2, 1, 2}},
      {{2, 2, 1}},
      {{2, 3, 2}},
      {{3, 2, 2}},
      {{1, 1, 2}},
      {{1, 2, 1}},
  }};

  for (const auto& neighbor : neighbors) {
    current.at(neighbor[0], neighbor[1], neighbor[2]) = 1;
  }

  step_sequential(current, next);

  const bool passed = next.at(2, 2, 2) == 0;
  return run_single_test("alive cell with more than six neighbors dies", passed, output);
}

}  // namespace

SequentialRunResult run_sequential(const SimulationConfig& config) {
  Grid3D initial_grid(config.size);
  initialize_grid_host(initial_grid, config.seed, config.alive_probability);
  return run_sequential_from_initial_grid(config, initial_grid);
}

SequentialRunResult run_sequential_from_initial_grid(const SimulationConfig& config, const Grid3D& initial_grid) {
  if (initial_grid.size() != config.size) {
    throw std::invalid_argument("Initial grid size does not match the configured simulation size.");
  }

  Grid3D current = initial_grid;
  Grid3D next(config.size);

  // Both buffers start with dead boundaries. Each step overwrites every interior cell,
  // so the fixed-dead boundary policy is preserved across buffer swaps.
  next.fill(0);

  SequentialRunResult result;
  result.initial_summary = summarize_grid(initial_grid);

  const auto start = std::chrono::steady_clock::now();

  for (int step = 0; step < config.steps; ++step) {
    step_sequential(current, next);
    current.swap(next);
  }

  const auto end = std::chrono::steady_clock::now();
  result.elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
  result.final_summary = summarize_grid(current);
  result.final_grid = std::move(current);

  return result;
}

bool run_sequential_sanity_checks(std::ostream& output) {
  output << "Running sequential sanity checks...\n";

  bool all_passed = true;
  all_passed = test_deterministic_boundary_initialization(output) && all_passed;
  all_passed = test_birth_rule(output) && all_passed;
  all_passed = test_underpopulation_rule(output) && all_passed;
  all_passed = test_survival_rule(output) && all_passed;
  all_passed = test_overpopulation_rule(output) && all_passed;

  output << (all_passed ? "All sequential sanity checks passed.\n" : "One or more sequential sanity checks failed.\n");
  return all_passed;
}

}  // namespace ca
