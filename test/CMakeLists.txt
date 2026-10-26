# Runtime suites and compile-only specification fixtures have separate roles.
add_executable(tusb32x_compile_contracts
  test_compile_contracts.cpp consumer/status_other_tu.cpp)
add_executable(tusb32x_validators test_validators.cpp)

foreach(suite IN ITEMS tusb32x_compile_contracts tusb32x_validators)
  target_link_libraries(${suite} PRIVATE TUSB32x)
  tusb32x_strict_warnings(${suite})
  if(MSVC)
    target_compile_options(${suite} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${suite} PRIVATE -UNDEBUG)
  endif()
  add_test(NAME ${suite} COMMAND ${suite})
  set_tests_properties(${suite} PROPERTIES TIMEOUT 30)
endforeach()

# Each header is the only include in its generated translation unit.
set(header_contract_sources)
foreach(header IN ITEMS Status Config TUSB32x CommandTable Version)
  set(source "${CMAKE_CURRENT_BINARY_DIR}/include_${header}.cpp")
  file(WRITE "${source}" "#include <TUSB32x/${header}.h>\n")
  list(APPEND header_contract_sources "${source}")
endforeach()
add_library(tusb32x_header_contracts OBJECT ${header_contract_sources}
  contract/planned_api.cpp contract/owner_examples.cpp contract/time_arithmetic.cpp)
target_link_libraries(tusb32x_header_contracts PRIVATE TUSB32x)
tusb32x_strict_warnings(tusb32x_header_contracts)
# No CTest entry: compiling planned signatures cannot prove driver behavior.

# Python is a maintainer test dependency only; tests-OFF consumers never find it.
find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
add_test(NAME tusb32x_version_sync
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_version_sync.py")
set_tests_properties(tusb32x_version_sync PROPERTIES TIMEOUT 120)
