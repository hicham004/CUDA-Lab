#include "ca/config.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

namespace ca {
namespace {

const char* require_value(int argc, char** argv, int& index, const std::string& option_name) {
  if (index + 1 >= argc) {
    throw std::invalid_argument("Missing value for " + option_name + ".");
  }

  ++index;
  return argv[index];
}

int parse_int(const std::string& value, const std::string& option_name) {
  try {
    std::size_t consumed = 0;
    const int parsed = std::stoi(value, &consumed);
    if (consumed != value.size()) {
      throw std::invalid_argument("Trailing characters.");
    }
    return parsed;
  } catch (const std::exception&) {
    throw std::invalid_argument("Invalid integer for " + option_name + ": " + value);
  }
}

std::uint32_t parse_uint32(const std::string& value, const std::string& option_name) {
  try {
    std::size_t consumed = 0;
    const unsigned long parsed = std::stoul(value, &consumed);
    if (consumed != value.size()) {
      throw std::invalid_argument("Trailing characters.");
    }
    return static_cast<std::uint32_t>(parsed);
  } catch (const std::exception&) {
    throw std::invalid_argument("Invalid unsigned integer for " + option_name + ": " + value);
  }
}

double parse_double(const std::string& value, const std::string& option_name) {
  try {
    std::size_t consumed = 0;
    const double parsed = std::stod(value, &consumed);
    if (consumed != value.size()) {
      throw std::invalid_argument("Trailing characters.");
    }
    return parsed;
  } catch (const std::exception&) {
    throw std::invalid_argument("Invalid floating-point value for " + option_name + ": " + value);
  }
}

void validate_config(const SimulationConfig& config, bool allow_cuda_flags) {
  if (config.size < 3) {
    throw std::invalid_argument("--size must be at least 3.");
  }

  if (config.steps < 0) {
    throw std::invalid_argument("--steps must be non-negative.");
  }

  if (config.alive_probability < 0.0 || config.alive_probability > 1.0) {
    throw std::invalid_argument("--alive-prob must be between 0.0 and 1.0.");
  }

  if (allow_cuda_flags) {
    if (config.block_x <= 0 || config.block_y <= 0 || config.block_z <= 0) {
      throw std::invalid_argument("CUDA block dimensions must all be positive.");
    }
  }
}

}  // namespace

SimulationConfig parse_arguments(int argc, char** argv, bool allow_cuda_flags) {
  SimulationConfig config;

  for (int index = 1; index < argc; ++index) {
    const std::string option = argv[index];

    if (option == "--help" || option == "-h") {
      config.show_help = true;
      continue;
    }

    if (option == "--sanity-check") {
      config.sanity_check = true;
      continue;
    }

    if (option == "--verify") {
      if (!allow_cuda_flags) {
        throw std::invalid_argument("--verify is only valid for the CUDA executable.");
      }
      config.verify = true;
      continue;
    }

    if (option == "--size") {
      config.size = parse_int(require_value(argc, argv, index, option), option);
      continue;
    }

    if (option == "--steps") {
      config.steps = parse_int(require_value(argc, argv, index, option), option);
      continue;
    }

    if (option == "--seed") {
      config.seed = parse_uint32(require_value(argc, argv, index, option), option);
      continue;
    }

    if (option == "--alive-prob") {
      config.alive_probability = parse_double(require_value(argc, argv, index, option), option);
      continue;
    }

    if (option == "--output-state") {
      config.output_state_path = require_value(argc, argv, index, option);
      continue;
    }

    if (option == "--metrics-out") {
      config.metrics_out_path = require_value(argc, argv, index, option);
      continue;
    }

    if (option == "--block-x") {
      if (!allow_cuda_flags) {
        throw std::invalid_argument("--block-x is only valid for the CUDA executable.");
      }
      config.block_x = parse_int(require_value(argc, argv, index, option), option);
      continue;
    }

    if (option == "--block-y") {
      if (!allow_cuda_flags) {
        throw std::invalid_argument("--block-y is only valid for the CUDA executable.");
      }
      config.block_y = parse_int(require_value(argc, argv, index, option), option);
      continue;
    }

    if (option == "--block-z") {
      if (!allow_cuda_flags) {
        throw std::invalid_argument("--block-z is only valid for the CUDA executable.");
      }
      config.block_z = parse_int(require_value(argc, argv, index, option), option);
      continue;
    }

    throw std::invalid_argument("Unknown argument: " + option);
  }

  validate_config(config, allow_cuda_flags);
  return config;
}

std::string usage_text(const char* program_name, bool include_cuda_flags) {
  std::ostringstream stream;
  const std::string resolved_name = program_name == nullptr ? "program" : program_name;

  stream << "Usage: " << resolved_name << " [options]\n\n";
  stream << "Common options:\n";
  stream << "  --size <int>          Grid size N for an N x N x N simulation.\n";
  stream << "  --steps <int>         Number of time steps.\n";
  stream << "  --seed <uint>         Fixed random seed.\n";
  stream << "  --alive-prob <float>  Initial alive probability.\n";
  stream << "  --output-state <path> Final-state output path.\n";
  stream << "  --metrics-out <path>  Metrics output path.\n";
  stream << "  --sanity-check        Run built-in sanity checks when supported.\n";
  stream << "  --help                Show this help message.\n";

  if (include_cuda_flags) {
    stream << "\nCUDA-only options:\n";
    stream << "  --block-x <int>       CUDA block dimension in x.\n";
    stream << "  --block-y <int>       CUDA block dimension in y.\n";
    stream << "  --block-z <int>       CUDA block dimension in z.\n";
    stream << "  --verify              Run exact final-state verification against the sequential result.\n";
  }

  return stream.str();
}

std::string summarize_config(const SimulationConfig& config, bool include_cuda_fields) {
  std::ostringstream stream;
  stream << "Configuration:\n";
  stream << "  size            = " << config.size << '\n';
  stream << "  steps           = " << config.steps << '\n';
  stream << "  seed            = " << config.seed << '\n';
  stream << "  alive_prob      = " << config.alive_probability << '\n';
  stream << "  sanity_check    = " << (config.sanity_check ? "true" : "false") << '\n';
  stream << "  output_state    = " << (config.output_state_path.empty() ? "<none>" : config.output_state_path) << '\n';
  stream << "  metrics_out     = " << (config.metrics_out_path.empty() ? "<none>" : config.metrics_out_path);

  if (include_cuda_fields) {
    stream << '\n';
    stream << "  block           = " << config.block_x << 'x' << config.block_y << 'x' << config.block_z << '\n';
    stream << "  verify          = " << (config.verify ? "true" : "false");
  }

  return stream.str();
}

}  // namespace ca
