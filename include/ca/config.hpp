#pragma once

#include <cstdint>
#include <string>

namespace ca {

struct SimulationConfig {
  int size = 64;
  int steps = 50;
  std::uint32_t seed = 12345;
  double alive_probability = 0.3;
  std::string output_state_path;
  std::string metrics_out_path;
  int block_x = 8;
  int block_y = 8;
  int block_z = 8;
  bool verify = false;
  bool sanity_check = false;
  bool show_help = false;
};

SimulationConfig parse_arguments(int argc, char** argv, bool allow_cuda_flags);
std::string usage_text(const char* program_name, bool include_cuda_flags);
std::string summarize_config(const SimulationConfig& config, bool include_cuda_fields);

}  // namespace ca
