#pragma once

#include "ca/config.hpp"
#include "ca/grid.hpp"
#include "ca/summary.hpp"

#include <iosfwd>

namespace ca {

struct SequentialRunResult {
  Grid3D final_grid;
  GridSummary initial_summary;
  GridSummary final_summary;
  double elapsed_ms = 0.0;
};

SequentialRunResult run_sequential(const SimulationConfig& config);
SequentialRunResult run_sequential_from_initial_grid(const SimulationConfig& config, const Grid3D& initial_grid);
bool run_sequential_sanity_checks(std::ostream& output);

}  // namespace ca
