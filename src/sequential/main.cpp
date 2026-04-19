#include "ca/config.hpp"
#include "ca/sequential.hpp"
#include "ca/summary.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
  try {
    const char* program_name = argc > 0 ? argv[0] : "ca_seq";
    const ca::SimulationConfig config = ca::parse_arguments(argc, argv, false);

    if (config.show_help) {
      std::cout << ca::usage_text(program_name, false);
      return 0;
    }

    if (config.sanity_check) {
      return ca::run_sequential_sanity_checks(std::cout) ? 0 : 1;
    }

    const ca::SequentialRunResult result = ca::run_sequential(config);

    if (!config.output_state_path.empty()) {
      ca::save_state_file(config.output_state_path, result.final_grid, config, result.final_summary);
    }

    if (!config.metrics_out_path.empty()) {
      ca::write_metrics_json(
          config.metrics_out_path,
          "sequential",
          config,
          result.initial_summary,
          result.final_summary,
          result.elapsed_ms,
          config.output_state_path);
    }

    std::cout << "Sequential CPU simulation complete.\n";
    std::cout << ca::summarize_config(config, false) << "\n\n";
    std::cout << "Initial summary:\n";
    std::cout << "  live_cells      = " << result.initial_summary.live_cells << '\n';
    std::cout << "  checksum        = " << ca::checksum_to_hex(result.initial_summary.checksum) << "\n\n";
    std::cout << "Final summary:\n";
    std::cout << "  live_cells      = " << result.final_summary.live_cells << '\n';
    std::cout << "  checksum        = " << ca::checksum_to_hex(result.final_summary.checksum) << "\n\n";
    std::cout << "Timing:\n";
    std::cout << "  elapsed_ms      = " << result.elapsed_ms << "\n\n";
    std::cout << "Outputs:\n";
    std::cout << "  final_state     = " << (config.output_state_path.empty() ? "<none>" : config.output_state_path) << '\n';
    std::cout << "  metrics_out     = " << (config.metrics_out_path.empty() ? "<none>" : config.metrics_out_path) << '\n';
    return 0;
  } catch (const std::exception& error) {
    const char* program_name = argc > 0 ? argv[0] : "ca_seq";
    std::cerr << error.what() << "\n\n";
    std::cerr << ca::usage_text(program_name, false);
    return 1;
  }
}
