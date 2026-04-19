#pragma once

#include "ca/grid.hpp"

#include <cstdint>

namespace ca {

void initialize_grid_host(Grid3D& grid, std::uint32_t seed, double alive_probability);

}  // namespace ca
