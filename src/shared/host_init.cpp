#include "ca/host_init.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace ca {
namespace {

constexpr std::uint64_t kProbabilityResolution = 1ULL << 53;

std::uint64_t splitmix64(std::uint64_t value) noexcept {
  value += 0x9e3779b97f4a7c15ULL;
  value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
  value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
  return value ^ (value >> 31U);
}

std::uint64_t probability_threshold(double alive_probability) {
  if (alive_probability <= 0.0) {
    return 0;
  }

  if (alive_probability >= 1.0) {
    return kProbabilityResolution;
  }

  const long double scaled = static_cast<long double>(alive_probability) * static_cast<long double>(kProbabilityResolution);
  return static_cast<std::uint64_t>(scaled);
}

Cell deterministic_cell_state(std::uint32_t seed, std::size_t linear_index, std::uint64_t threshold) noexcept {
  const std::uint64_t mixed_seed = (static_cast<std::uint64_t>(seed) << 32U) ^ static_cast<std::uint64_t>(linear_index);
  const std::uint64_t sample = splitmix64(mixed_seed) >> 11U;
  return sample < threshold ? static_cast<Cell>(1) : static_cast<Cell>(0);
}

}  // namespace

void initialize_grid_host(Grid3D& grid, std::uint32_t seed, double alive_probability) {
  if (!std::isfinite(alive_probability) || alive_probability < 0.0 || alive_probability > 1.0) {
    throw std::invalid_argument("Alive probability must be a finite value between 0.0 and 1.0.");
  }

  const int size = grid.size();
  const std::uint64_t threshold = probability_threshold(alive_probability);

  grid.fill(0);

  for (int i = 1; i < size - 1; ++i) {
    for (int j = 1; j < size - 1; ++j) {
      for (int k = 1; k < size - 1; ++k) {
        const std::size_t index = grid.linear_index(i, j, k);
        grid.at(i, j, k) = deterministic_cell_state(seed, index, threshold);
      }
    }
  }
}

}  // namespace ca
