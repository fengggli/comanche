cmake_minimum_required (VERSION 3.5.1 FATAL_ERROR)

project(cuda-dma LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda/)

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -std=c++11 -O3 -DCONFIG_DEBUG -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_70,code=sm_70 -gencode arch=compute_70,code=compute_70)
set(CUDA_HOST_COMPILATION_CPP ON)
set(CUDA_SEPARABLE_COMPILATION ON)
set(CUDA_VERBOSE ON)
set(CUDA_64_BIT_DEVICE_CODE ON CACHE STRING "Compile device code in 64 bit mode" FORCE)

message(STATUS "FoundCUDA              : ${CUDA_FOUND}")
message(STATUS "Cuda cublas libraries  : ${CUDA_CUBLAS_LIBRARIES}")

cuda_include_directories($ENV{COMANCHE_HOME}/deps/gdrcopy/)
cuda_include_directories(${CMAKE_INSTALL_PREFIX}/include)
cuda_include_directories($ENV{COMANCHE_HOME}/src/components/)

# build cuda code into static library

cuda_add_library(cuda_app_lib src/kernel.cu)
include_directories(${CMAKE_INSTALL_PREFIX}/include)
link_directories(/usr/local/cuda/lib64/)
link_directories(${CMAKE_INSTALL_PREFIX}/lib64)

file(GLOB SOURCES src/*.cpp)

add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
add_executable(cuda-dma ${SOURCES})

target_link_libraries(cuda-dma cuda_app_lib ${ASAN_LIB} common comanche-core numa pthread dl z boost_system boost_program_options cudadevrt gdrapi)

