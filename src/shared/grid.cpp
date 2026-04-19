#include "ca/grid.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace ca {

Grid3D::Grid3D(int size) : size_(size) {
  if (size <= 0) {
    throw std::invalid_argument("Grid size must be positive.");
  }

  cells_.assign(static_cast<std::size_t>(size) * size * size, 0);
}

Cell& Grid3D::at(int i, int j, int k) noexcept {
  return cells_[linear_index(i, j, k)];
}

const Cell& Grid3D::at(int i, int j, int k) const noexcept {
  return cells_[linear_index(i, j, k)];
}

void Grid3D::fill(Cell value) noexcept {
  std::fill(cells_.begin(), cells_.end(), value);
}

void Grid3D::swap(Grid3D& other) noexcept {
  std::swap(size_, other.size_);
  cells_.swap(other.cells_);
}

std::size_t Grid3D::linear_index(int size, int i, int j, int k) noexcept {
  return linear_index_3d(size, i, j, k);
}

std::size_t Grid3D::linear_index(int i, int j, int k) const noexcept {
  return linear_index(size_, i, j, k);
}

bool Grid3D::is_boundary(int size, int i, int j, int k) noexcept {
  return is_boundary_cell(size, i, j, k);
}

}  // namespace ca
