#include "ca/cuda.hpp"

#include "ca/host_init.hpp"
#include "ca/sequential.hpp"
#include "ca/simulation_common.hpp"

#include <cuda_runtime.h>

#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace ca {
namespace {

class DeviceBuffer {
 public:
  DeviceBuffer() = default;

  explicit DeviceBuffer(std::size_t byte_count) : byte_count_(byte_count) {
    allocate(byte_count);
  }

  ~DeviceBuffer() {
    release();
  }

  DeviceBuffer(const DeviceBuffer&) = delete;
  DeviceBuffer& operator=(const DeviceBuffer&) = delete;

  DeviceBuffer(DeviceBuffer&& other) noexcept : pointer_(other.pointer_), byte_count_(other.byte_count_) {
    other.pointer_ = nullptr;
    other.byte_count_ = 0;
  }

  DeviceBuffer& operator=(DeviceBuffer&& other) noexcept {
    if (this != &other) {
      release();
      pointer_ = other.pointer_;
      byte_count_ = other.byte_count_;
      other.pointer_ = nullptr;
      other.byte_count_ = 0;
    }

    return *this;
  }

  Cell* data() noexcept { return pointer_; }
  const Cell* data() const noexcept { return pointer_; }
  std::size_t byte_count() const noexcept { return byte_count_; }

 private:
  void allocate(std::size_t byte_count) {
    if (byte_count == 0) {
      return;
    }

    Cell* pointer = nullptr;
    const cudaError_t error = cudaMalloc(reinterpret_cast<void**>(&pointer), byte_count);
    if (error != cudaSuccess) {
      std::ostringstream message;
      message << "cudaMalloc failed: " << cudaGetErrorString(error);
      throw std::runtime_error(message.str());
    }

    pointer_ = pointer;
    byte_count_ = byte_count;
  }

  void release() noexcept {
    if (pointer_ != nullptr) {
      cudaFree(pointer_);
      pointer_ = nullptr;
      byte_count_ = 0;
    }
  }

  Cell* pointer_ = nullptr;
  std::size_t byte_count_ = 0;
};

void throw_if_cuda_failed(cudaError_t error, const std::string& context) {
  if (error == cudaSuccess) {
    return;
  }

  std::ostringstream message;
  message << context << " failed: " << cudaGetErrorString(error);
  throw std::runtime_error(message.str());
}

void validate_block_configuration(const SimulationConfig& config, const cudaDeviceProp& properties) {
  const int threads_per_block = config.block_x * config.block_y * config.block_z;

  if (config.block_x > properties.maxThreadsDim[0] ||
      config.block_y > properties.maxThreadsDim[1] ||
      config.block_z > properties.maxThreadsDim[2]) {
    throw std::invalid_argument("Requested CUDA block dimensions exceed the device limits.");
  }

  if (threads_per_block > properties.maxThreadsPerBlock) {
    throw std::invalid_argument("Requested CUDA block uses more threads than the device allows per block.");
  }
}

__global__ void step_kernel(const Cell* current, Cell* next, int size) {
  const int i = static_cast<int>(blockIdx.x * blockDim.x + threadIdx.x);
  const int j = static_cast<int>(blockIdx.y * blockDim.y + threadIdx.y);
  const int k = static_cast<int>(blockIdx.z * blockDim.z + threadIdx.z);

  if (i >= size || j >= size || k >= size) {
    return;
  }

  const std::size_t index = linear_index_3d(size, i, j, k);

  if (is_boundary_cell(size, i, j, k)) {
    next[index] = 0;
    return;
  }

  int live_neighbors = 0;

  for (int di = -1; di <= 1; ++di) {
    for (int dj = -1; dj <= 1; ++dj) {
      for (int dk = -1; dk <= 1; ++dk) {
        if (di == 0 && dj == 0 && dk == 0) {
          continue;
        }

        const std::size_t neighbor_index = linear_index_3d(size, i + di, j + dj, k + dk);
        live_neighbors += current[neighbor_index] != 0 ? 1 : 0;
      }
    }
  }

  next[index] = apply_rule(current[index], live_neighbors);
}

}  // namespace

CudaRunResult run_cuda(const SimulationConfig& config) {
  Grid3D initial_grid(config.size);
  initialize_grid_host(initial_grid, config.seed, config.alive_probability);

  CudaRunResult result;
  result.initial_summary = summarize_grid(initial_grid);
  result.verification_requested = config.verify;

  cudaDeviceProp device_properties{};
  throw_if_cuda_failed(cudaGetDeviceProperties(&device_properties, 0), "cudaGetDeviceProperties");
  validate_block_configuration(config, device_properties);

  const std::size_t byte_count = initial_grid.cell_count() * sizeof(Cell);
  DeviceBuffer current_device(byte_count);
  DeviceBuffer next_device(byte_count);

  throw_if_cuda_failed(
      cudaMemcpy(current_device.data(), initial_grid.data(), byte_count, cudaMemcpyHostToDevice),
      "cudaMemcpy host-to-device");
  throw_if_cuda_failed(cudaMemset(next_device.data(), 0, byte_count), "cudaMemset next buffer");

  const dim3 block_dimensions(
      static_cast<unsigned int>(config.block_x),
      static_cast<unsigned int>(config.block_y),
      static_cast<unsigned int>(config.block_z));
  const dim3 grid_dimensions(
      static_cast<unsigned int>((config.size + config.block_x - 1) / config.block_x),
      static_cast<unsigned int>((config.size + config.block_y - 1) / config.block_y),
      static_cast<unsigned int>((config.size + config.block_z - 1) / config.block_z));

  throw_if_cuda_failed(cudaDeviceSynchronize(), "cudaDeviceSynchronize before timing");
  const auto start = std::chrono::steady_clock::now();

  for (int step = 0; step < config.steps; ++step) {
    step_kernel<<<grid_dimensions, block_dimensions>>>(current_device.data(), next_device.data(), config.size);
    throw_if_cuda_failed(cudaGetLastError(), "step_kernel launch");
    std::swap(current_device, next_device);
  }

  throw_if_cuda_failed(cudaDeviceSynchronize(), "cudaDeviceSynchronize after timing");
  const auto end = std::chrono::steady_clock::now();
  result.elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();

  result.final_grid = Grid3D(config.size);
  throw_if_cuda_failed(
      cudaMemcpy(result.final_grid.data(), current_device.data(), byte_count, cudaMemcpyDeviceToHost),
      "cudaMemcpy device-to-host");
  result.final_summary = summarize_grid(result.final_grid);

  if (config.verify) {
    const SequentialRunResult reference_result = run_sequential_from_initial_grid(config, initial_grid);
    result.reference_summary = reference_result.final_summary;
    result.reference_elapsed_ms = reference_result.elapsed_ms;
    result.verification = compare_grids_exact(reference_result.final_grid, result.final_grid);
  }

  return result;
}

}  // namespace ca
