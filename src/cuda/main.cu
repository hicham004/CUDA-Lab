#include "ca/config.hpp"
#include "ca/cuda.hpp"
#include "ca/summary.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
  try {
    const char* program_name = argc > 0 ? argv[0] : "ca_cuda";
    ca::SimulationConfig config = ca::parse_arguments(argc, argv, true);

    if (config.show_help) {
      std::cout << ca::usage_text(program_name, true);
      return 0;
    }

    if (config.sanity_check) {
      config.verify = true;
    }

    const ca::CudaRunResult result = ca::run_cuda(config);

    if (!config.output_state_path.empty()) {
      ca::save_state_file(config.output_state_path, result.final_grid, config, result.final_summary);
    }

    if (!config.metrics_out_path.empty()) {
      ca::write_metrics_json(
          config.metrics_out_path,
          "cuda",
          config,
          result.initial_summary,
          result.final_summary,
          result.elapsed_ms,
          config.output_state_path);
    }

    std::cout << "CUDA simulation complete.\n";
    std::cout << ca::summarize_config(config, true) << "\n\n";
    std::cout << "Initial summary:\n";
    std::cout << "  live_cells      = " << result.initial_summary.live_cells << '\n';
    std::cout << "  checksum        = " << ca::checksum_to_hex(result.initial_summary.checksum) << "\n\n";
    std::cout << "Final summary:\n";
    std::cout << "  live_cells      = " << result.final_summary.live_cells << '\n';
    std::cout << "  checksum        = " << ca::checksum_to_hex(result.final_summary.checksum) << "\n\n";
    std::cout << "Timing:\n";
    std::cout << "  elapsed_ms      = " << result.elapsed_ms << "\n\n";

    std::cout << "Verification:\n";
    if (!result.verification_requested) {
      std::cout << "  status          = not_requested\n";
    } else if (result.verification.passed) {
      std::cout << "  status          = PASS\n";
      std::cout << "  mismatches      = 0\n";
      std::cout << "  seq_checksum    = " << ca::checksum_to_hex(result.reference_summary.checksum) << '\n';
      std::cout << "  seq_elapsed_ms  = " << result.reference_elapsed_ms << '\n';
    } else {
      std::cout << "  status          = FAIL\n";
      std::cout << "  mismatches      = " << result.verification.mismatch_count << '\n';
      std::cout << "  first_mismatch  = (" << result.verification.first_i << ", " << result.verification.first_j << ", "
                << result.verification.first_k << ")\n";
      std::cout << "  expected_value  = " << static_cast<int>(result.verification.expected_value) << '\n';
      std::cout << "  actual_value    = " << static_cast<int>(result.verification.actual_value) << '\n';
      std::cout << "  seq_checksum    = " << ca::checksum_to_hex(result.reference_summary.checksum) << '\n';
      std::cout << "  seq_elapsed_ms  = " << result.reference_elapsed_ms << '\n';
      return 2;
    }

    std::cout << "\nOutputs:\n";
    std::cout << "  final_state     = " << (config.output_state_path.empty() ? "<none>" : config.output_state_path) << '\n';
    std::cout << "  metrics_out     = " << (config.metrics_out_path.empty() ? "<none>" : config.metrics_out_path) << '\n';
    return 0;
  } catch (const std::exception& error) {
    const char* program_name = argc > 0 ? argv[0] : "ca_cuda";
    std::cerr << error.what() << "\n\n";
    std::cerr << ca::usage_text(program_name, true);
    return 1;
  }
}
