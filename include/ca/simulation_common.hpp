#pragma once

#include <cstddef>
#include <cstdint>

#if defined(__CUDACC__)
#define CA_HOST_DEVICE __host__ __device__
#else
#define CA_HOST_DEVICE
#endif

namespace ca {

using Cell = std::uint8_t;

CA_HOST_DEVICE inline std::size_t linear_index_3d(int size, int i, int j, int k) noexcept {
  const std::size_t n = static_cast<std::size_t>(size);
  return static_cast<std::size_t>(i) * n * n + static_cast<std::size_t>(j) * n + static_cast<std::size_t>(k);
}

CA_HOST_DEVICE inline bool is_boundary_cell(int size, int i, int j, int k) noexcept {
  return i == 0 || j == 0 || k == 0 || i == size - 1 || j == size - 1 || k == size - 1;
}

CA_HOST_DEVICE inline Cell apply_rule(Cell current_state, int live_neighbors) noexcept {
  if (current_state == 0) {
    return live_neighbors == 5 ? static_cast<Cell>(1) : static_cast<Cell>(0);
  }

  return (live_neighbors < 4 || live_neighbors > 6) ? static_cast<Cell>(0) : static_cast<Cell>(1);
}

}  // namespace ca

#undef CA_HOST_DEVICE
