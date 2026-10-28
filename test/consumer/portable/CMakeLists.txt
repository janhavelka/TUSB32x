cmake_minimum_required(VERSION 3.16)
project(TUSB32xPortableConsumer LANGUAGES CXX)

# Deliberately import a previously built library: no parent project or tools run here.
set(TUSB32X_PUBLIC_INCLUDE_DIR "" CACHE PATH "Directory containing public TUSB32x headers")
set(TUSB32X_LIBRARY_FILE "" CACHE FILEPATH "Previously built TUSB32x static library")
if(NOT EXISTS "${TUSB32X_PUBLIC_INCLUDE_DIR}/TUSB32x/TUSB32x.h" OR
   NOT EXISTS "${TUSB32X_LIBRARY_FILE}")
  message(FATAL_ERROR "Set TUSB32X_PUBLIC_INCLUDE_DIR and TUSB32X_LIBRARY_FILE to existing artifacts")
endif()
add_library(consumer_library STATIC IMPORTED)
set_target_properties(consumer_library PROPERTIES
  IMPORTED_LOCATION "${TUSB32X_LIBRARY_FILE}"
  INTERFACE_INCLUDE_DIRECTORIES "${TUSB32X_PUBLIC_INCLUDE_DIR}")
add_executable(tusb32x_portable_consumer main.cpp secondary.cpp)
target_link_libraries(tusb32x_portable_consumer PRIVATE consumer_library)
target_compile_features(tusb32x_portable_consumer PRIVATE cxx_std_17)
set_target_properties(tusb32x_portable_consumer PROPERTIES CXX_EXTENSIONS OFF)
if(MSVC)
  target_compile_options(tusb32x_portable_consumer PRIVATE /W4 /WX /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(tusb32x_portable_consumer PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)
endif()

# GNU-style wrapping also catches C allocator references from the static core.
# The probe is a compile/link check; supported hosts additionally execute a sentinel.
include(CheckCXXSourceCompiles)
option(TUSB32X_WRAP_C_ALLOCATIONS "Wrap C allocator references when supported by the linker" ON)
if(TUSB32X_WRAP_C_ALLOCATIONS)
  set(CMAKE_REQUIRED_LINK_OPTIONS "-Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free")
  check_cxx_source_compiles("#include <cstdlib>
extern \"C\" void* __real_malloc(std::size_t);
extern \"C\" void* __real_calloc(std::size_t, std::size_t);
extern \"C\" void* __real_realloc(void*, std::size_t);
extern \"C\" void __real_free(void*);
extern \"C\" void* __wrap_malloc(std::size_t n) { return __real_malloc(n); }
extern \"C\" void* __wrap_calloc(std::size_t n, std::size_t s) { return __real_calloc(n, s); }
extern \"C\" void* __wrap_realloc(void* p, std::size_t n) { return __real_realloc(p, n); }
extern \"C\" void __wrap_free(void* p) { __real_free(p); }
int main() { std::free(std::malloc(1)); }" TUSB32X_CONSUMER_C_WRAP)
  unset(CMAKE_REQUIRED_LINK_OPTIONS)
endif()
if(TUSB32X_WRAP_C_ALLOCATIONS AND TUSB32X_CONSUMER_C_WRAP)
  target_compile_definitions(tusb32x_portable_consumer PRIVATE TUSB32X_CONSUMER_C_WRAP=1)
  target_link_options(tusb32x_portable_consumer PRIVATE
    -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free)
endif()
