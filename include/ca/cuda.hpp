#pragma once

#include "ca/config.hpp"
#include "ca/grid.hpp"
#include "ca/summary.hpp"
#include "ca/verification.hpp"

namespace ca {

struct CudaRunResult {
  Grid3D final_grid;
  GridSummary initial_summary;
  GridSummary final_summary;
  double elapsed_ms = 0.0;
  bool verification_requested = false;
  VerificationResult verification;
  GridSummary reference_summary;
  double reference_elapsed_ms = 0.0;
};

CudaRunResult run_cuda(const SimulationConfig& config);

}  // namespace ca
