cmake_minimum_required(VERSION 3.16)
project(TUSB32xHostBasic LANGUAGES CXX)

# The library supplies its public include path and C++17 requirement.
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../.." "${CMAKE_CURRENT_BINARY_DIR}/tusb32x")
if(NOT TARGET tusb32x_host_basic)
  add_executable(tusb32x_host_basic main.cpp profile.cpp)
  target_link_libraries(tusb32x_host_basic PRIVATE TUSB32x)
  tusb32x_strict_warnings(tusb32x_host_basic)
endif()
set_target_properties(tusb32x_host_basic PROPERTIES CXX_EXTENSIONS OFF)

# Tests ON already creates this executable; expose its tests at this project root too.
if(TUSB32X_BUILD_TESTS)
  enable_testing()
endif()
