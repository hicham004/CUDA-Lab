cmake_minimum_required(VERSION 3.24)

project(cuda_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config Debug Release RelWithDebInfo MinSizeRel)
  string(TOUPPER "${config}" config_upper)
  set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${CMAKE_BINARY_DIR}/bin/${config}")
endforeach()

option(CUDA_LAB_ENABLE_CUDA "Build the CUDA executable when a CUDA compiler is available." ON)

add_library(ca_common STATIC
  src/shared/config.cpp
  src/shared/grid.cpp
  src/shared/host_init.cpp
  src/shared/summary.cpp
  src/shared/verification.cpp
)

target_include_directories(ca_common PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(ca_common PUBLIC cxx_std_17)

add_executable(ca_seq
  src/sequential/simulation.cpp
  src/sequential/main.cpp
)

target_link_libraries(ca_seq PRIVATE ca_common)

include(CheckLanguage)
if(CUDA_LAB_ENABLE_CUDA)
  check_language(CUDA)
  if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)

    add_executable(ca_cuda
      src/cuda/simulation.cu
      src/cuda/main.cu
    )

    target_link_libraries(ca_cuda PRIVATE ca_common)
    set_target_properties(ca_cuda PROPERTIES
      CUDA_STANDARD 17
      CUDA_STANDARD_REQUIRED ON
      CUDA_SEPARABLE_COMPILATION ON
    )

    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
      message(STATUS "CMAKE_CUDA_ARCHITECTURES is not set. Pass -DCMAKE_CUDA_ARCHITECTURES=<arch> or native when configuring.")
    endif()
  else()
    message(STATUS "CUDA compiler not found. Skipping ca_cuda target.")
  endif()
endif()
