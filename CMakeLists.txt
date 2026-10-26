# Portable values and local validators; package/version metadata arrives separately.
cmake_minimum_required(VERSION 3.16)
project(TUSB32x LANGUAGES CXX)

option(TUSB32X_BUILD_TESTS "Build the portable contract and validator checks" OFF)

add_library(TUSB32x STATIC src/TUSB32x.cpp)
target_compile_features(TUSB32x PUBLIC cxx_std_17)
target_include_directories(TUSB32x PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(TUSB32x PROPERTIES CXX_EXTENSIONS OFF)

# Warnings are private: applications choose their own diagnostic policy.
function(tusb32x_strict_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX /permissive-)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)
  endif()
endfunction()
tusb32x_strict_warnings(TUSB32x)

if(TUSB32X_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
