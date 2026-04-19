#pragma once

#include "ca/config.hpp"
#include "ca/grid.hpp"

#include <cstdint>
#include <string>

namespace ca {

struct GridSummary {
  std::uint64_t live_cells = 0;
  std::uint64_t checksum = 0;
};

GridSummary summarize_grid(const Grid3D& grid);
std::string checksum_to_hex(std::uint64_t checksum);

void save_state_file(
    const std::string& path,
    const Grid3D& grid,
    const SimulationConfig& config,
    const GridSummary& summary);

void write_metrics_json(
    const std::string& path,
    const std::string& implementation_name,
    const SimulationConfig& config,
    const GridSummary& initial_summary,
    const GridSummary& final_summary,
    double elapsed_ms,
    const std::string& output_state_path);

}  // namespace ca
