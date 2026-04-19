#pragma once

#include "ca/simulation_common.hpp"

#include <cstddef>
#include <vector>

namespace ca {

class Grid3D {
 public:
  Grid3D() = default;
  explicit Grid3D(int size);

  int size() const noexcept { return size_; }
  std::size_t cell_count() const noexcept { return cells_.size(); }

  Cell* data() noexcept { return cells_.data(); }
  const Cell* data() const noexcept { return cells_.data(); }

  Cell& at(int i, int j, int k) noexcept;
  const Cell& at(int i, int j, int k) const noexcept;

  void fill(Cell value) noexcept;
  void swap(Grid3D& other) noexcept;

  static std::size_t linear_index(int size, int i, int j, int k) noexcept;
  std::size_t linear_index(int i, int j, int k) const noexcept;
  static bool is_boundary(int size, int i, int j, int k) noexcept;

 private:
  int size_ = 0;
  std::vector<Cell> cells_;
};

}  // namespace ca
