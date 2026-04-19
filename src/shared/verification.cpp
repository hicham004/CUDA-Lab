#include "ca/verification.hpp"

namespace ca {

VerificationResult compare_grids_exact(const Grid3D& expected, const Grid3D& actual) {
  VerificationResult result;

  if (expected.size() != actual.size()) {
    result.passed = false;
    result.mismatch_count = 1;
    return result;
  }

  const int size = expected.size();

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      for (int k = 0; k < size; ++k) {
        const Cell expected_value = expected.at(i, j, k);
        const Cell actual_value = actual.at(i, j, k);

        if (expected_value == actual_value) {
          continue;
        }

        if (result.mismatch_count == 0) {
          result.first_i = i;
          result.first_j = j;
          result.first_k = k;
          result.expected_value = expected_value;
          result.actual_value = actual_value;
        }

        ++result.mismatch_count;
      }
    }
  }

  result.passed = result.mismatch_count == 0;
  return result;
}

}  // namespace ca
