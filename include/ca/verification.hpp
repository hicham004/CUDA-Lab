#pragma once

#include "ca/grid.hpp"

#include <cstdint>

namespace ca {

struct VerificationResult {
  bool passed = false;
  std::uint64_t mismatch_count = 0;
  int first_i = -1;
  int first_j = -1;
  int first_k = -1;
  Cell expected_value = 0;
  Cell actual_value = 0;
};

VerificationResult compare_grids_exact(const Grid3D& expected, const Grid3D& actual);

}  // namespace ca
