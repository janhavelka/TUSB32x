# Checked-in version fields are synchronized by scripts/generate_version.py.
cmake_minimum_required(VERSION 3.16)
set(TUSB32X_VERSION "0.1.0-dev.1")
set(TUSB32X_SOURCES src/TUSB32x.cpp)

# Warnings are private: applications choose their own diagnostic policy.
function(tusb32x_strict_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX /permissive-)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)
  endif()
endfunction()

if(COMMAND idf_component_register)
  idf_component_register(SRCS ${TUSB32X_SOURCES} INCLUDE_DIRS "include")
  target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
  set_target_properties(${COMPONENT_LIB} PROPERTIES CXX_EXTENSIONS OFF)
  tusb32x_strict_warnings(${COMPONENT_LIB})
  return()
endif()

# CMake project VERSION accepts the numeric core, while TUSB32X_VERSION keeps SemVer suffixes.
project(TUSB32x VERSION 0.1.0 LANGUAGES CXX)
option(TUSB32X_BUILD_TESTS "Build the portable contract and validator checks" OFF)
add_library(TUSB32x STATIC ${TUSB32X_SOURCES})
target_compile_features(TUSB32x PUBLIC cxx_std_17)
target_include_directories(TUSB32x PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(TUSB32x PROPERTIES CXX_EXTENSIONS OFF)
tusb32x_strict_warnings(TUSB32x)

if(TUSB32X_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
