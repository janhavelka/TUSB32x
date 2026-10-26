cmake_minimum_required(VERSION 3.16)
project(TUSB32xHostBasic LANGUAGES CXX)

# The library supplies its public include path and C++17 requirement.
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../.." "${CMAKE_CURRENT_BINARY_DIR}/tusb32x")
add_executable(tusb32x_host_basic main.cpp profile.cpp)
target_link_libraries(tusb32x_host_basic PRIVATE TUSB32x)
