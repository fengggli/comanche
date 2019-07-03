cmake_minimum_required (VERSION 3.5.1 FATAL_ERROR)
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS} -DCONFIG_DEBUG)
include_directories(${CMAKE_SOURCE_DIR}/mcas/src/kernel/modules/mcas/)
link_directories(/usr/lib/x86_64-linux-gnu/)
add_executable(test-mcas-nvmestore testcli.cc)
target_link_libraries(test-mcas-nvmestore ${ASAN_LIB} common comanche-core numa pthread gtest dl  nupm boost_program_options)

add_executable(test-vfio test-vfio.c)
