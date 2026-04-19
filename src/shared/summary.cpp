#include "ca/summary.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>

namespace ca {
namespace {

constexpr std::uint64_t kFnvOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

void ensure_parent_directory(const std::string& path) {
  if (path.empty()) {
    return;
  }

  const std::filesystem::path filesystem_path(path);
  if (filesystem_path.has_parent_path()) {
    std::filesystem::create_directories(filesystem_path.parent_path());
  }
}

template <typename T>
void write_binary_value(std::ostream& output, const T& value) {
  output.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

std::string escape_json(const std::string& value) {
  std::ostringstream output;

  for (const char character : value) {
    switch (character) {
      case '\\':
        output << "\\\\";
        break;
      case '"':
        output << "\\\"";
        break;
      case '\n':
        output << "\\n";
        break;
      case '\r':
        output << "\\r";
        break;
      case '\t':
        output << "\\t";
        break;
      default:
        output << character;
        break;
    }
  }

  return output.str();
}

}  // namespace

GridSummary summarize_grid(const Grid3D& grid) {
  GridSummary summary;
  std::uint64_t checksum = kFnvOffsetBasis;

  const int size = grid.size();
  checksum ^= static_cast<std::uint64_t>(size);
  checksum *= kFnvPrime;

  for (std::size_t index = 0; index < grid.cell_count(); ++index) {
    const Cell value = grid.data()[index];
    summary.live_cells += static_cast<std::uint64_t>(value != 0);
    checksum ^= static_cast<std::uint64_t>(value);
    checksum *= kFnvPrime;
  }

  summary.checksum = checksum;
  return summary;
}

std::string checksum_to_hex(std::uint64_t checksum) {
  std::ostringstream output;
  output << "0x" << std::hex << std::setw(16) << std::setfill('0') << checksum;
  return output.str();
}

void save_state_file(
    const std::string& path,
    const Grid3D& grid,
    const SimulationConfig& config,
    const GridSummary& summary) {
  ensure_parent_directory(path);

  std::ofstream output(path, std::ios::binary);
  if (!output) {
    throw std::runtime_error("Failed to open final-state output file: " + path);
  }

  const char magic[8] = {'C', 'A', '3', 'D', 'G', 'R', 'D', '1'};
  const std::uint32_t version = 1;
  const std::uint32_t size = static_cast<std::uint32_t>(config.size);
  const std::uint32_t steps = static_cast<std::uint32_t>(config.steps);
  const std::uint64_t cell_count = static_cast<std::uint64_t>(grid.cell_count());

  output.write(magic, sizeof(magic));
  write_binary_value(output, version);
  write_binary_value(output, size);
  write_binary_value(output, steps);
  write_binary_value(output, config.seed);
  write_binary_value(output, config.alive_probability);
  write_binary_value(output, summary.live_cells);
  write_binary_value(output, summary.checksum);
  write_binary_value(output, cell_count);
  output.write(reinterpret_cast<const char*>(grid.data()), static_cast<std::streamsize>(grid.cell_count()));

  if (!output) {
    throw std::runtime_error("Failed while writing final-state output file: " + path);
  }
}

void write_metrics_json(
    const std::string& path,
    const std::string& implementation_name,
    const SimulationConfig& config,
    const GridSummary& initial_summary,
    const GridSummary& final_summary,
    double elapsed_ms,
    const std::string& output_state_path) {
  ensure_parent_directory(path);

  std::ofstream output(path, std::ios::binary);
  if (!output) {
    throw std::runtime_error("Failed to open metrics output file: " + path);
  }

  output << "{\n";
  output << "  \"implementation\": \"" << escape_json(implementation_name) << "\",\n";
  output << "  \"size\": " << config.size << ",\n";
  output << "  \"steps\": " << config.steps << ",\n";
  output << "  \"seed\": " << config.seed << ",\n";
  output << "  \"alive_probability\": " << std::setprecision(17) << config.alive_probability << ",\n";
  output << "  \"boundary_policy\": \"fixed_dead\",\n";
  output << "  \"initial_live_cells\": " << initial_summary.live_cells << ",\n";
  output << "  \"initial_checksum\": \"" << checksum_to_hex(initial_summary.checksum) << "\",\n";
  output << "  \"final_live_cells\": " << final_summary.live_cells << ",\n";
  output << "  \"final_checksum\": \"" << checksum_to_hex(final_summary.checksum) << "\",\n";
  output << "  \"elapsed_ms\": " << std::setprecision(17) << elapsed_ms << ",\n";
  output << "  \"output_state_path\": \"" << escape_json(output_state_path) << "\"\n";
  output << "}\n";

  if (!output) {
    throw std::runtime_error("Failed while writing metrics output file: " + path);
  }
}

}  // namespace ca
