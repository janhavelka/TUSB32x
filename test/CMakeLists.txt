# Runtime suites and compile-only specification fixtures have separate roles.
add_executable(tusb32x_compile_contracts
  test_compile_contracts.cpp consumer/status_other_tu.cpp)
add_executable(tusb32x_validators test_validators.cpp)
add_executable(tusb32x_protocol test_protocol.cpp test_codecs.cpp)
target_include_directories(tusb32x_protocol PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")
add_executable(tusb32x_operations test_operations.cpp test_profile_observation.cpp
  test_profile_application.cpp test_reset_recovery.cpp test_protocol_conformance.cpp
  test_owner_scheduling.cpp)
target_include_directories(tusb32x_operations PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")
add_executable(tusb32x_port test_port.cpp test_interrupt.cpp)
target_include_directories(tusb32x_port PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")
add_executable(tusb32x_diagnostics test_diagnostics.cpp)
target_include_directories(tusb32x_diagnostics PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")

# Run the same ordinary consumer shipped in the package, without framework/CLI sources.
add_executable(tusb32x_host_basic ../examples/host/basic/main.cpp ../examples/host/basic/profile.cpp)

# Example-only code is linked by CLI tests/fixture, never by the core target.
add_library(tusb32x_cli_support STATIC
  ../examples/common/Tusb32xCli.cpp ../examples/common/Tusb32xCliOutput.cpp)
target_link_libraries(tusb32x_cli_support PUBLIC TUSB32x)
target_include_directories(tusb32x_cli_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../examples/common")
tusb32x_strict_warnings(tusb32x_cli_support)
add_executable(tusb32x_cli cli/test_cli.cpp cli/test_cli_operations.cpp cli/test_cli_diagnostics.cpp)
target_link_libraries(tusb32x_cli PRIVATE tusb32x_cli_support)
add_executable(tusb32x_console console/test_console.cpp)
add_executable(tusb32x_example_owner test_example_owner.cpp
  ../examples/common/ExampleOwner.cpp)
target_include_directories(tusb32x_example_owner PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../examples/common")
add_executable(tusb32x_example_integration test_example_integration.cpp
  ../examples/common/ExampleOwner.cpp)
target_link_libraries(tusb32x_example_integration PRIVATE tusb32x_cli_support)
add_executable(tusb32x_output_mailbox test_output_mailbox.cpp)
find_package(Threads REQUIRED)
target_link_libraries(tusb32x_output_mailbox PRIVATE Threads::Threads)
target_include_directories(tusb32x_output_mailbox PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../examples/common")
target_include_directories(tusb32x_console PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../examples/common")
target_compile_features(tusb32x_console PRIVATE cxx_std_17)
add_executable(tusb32x_cli_host cli/cli_host.cpp)
target_link_libraries(tusb32x_cli_host PRIVATE tusb32x_cli_support)
tusb32x_strict_warnings(tusb32x_cli_host)

foreach(suite IN ITEMS tusb32x_compile_contracts tusb32x_validators tusb32x_protocol
    tusb32x_operations tusb32x_port tusb32x_diagnostics tusb32x_cli tusb32x_console
    tusb32x_example_owner tusb32x_example_integration tusb32x_output_mailbox tusb32x_host_basic)
  if(NOT suite STREQUAL "tusb32x_console")
    target_link_libraries(${suite} PRIVATE TUSB32x)
  endif()
  tusb32x_strict_warnings(${suite})
  if(MSVC)
    target_compile_options(${suite} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${suite} PRIVATE -UNDEBUG)
  endif()
  add_test(NAME ${suite} COMMAND ${suite})
  set_tests_properties(${suite} PROPERTIES TIMEOUT 30)
endforeach()

# Compile the real example adapter against recording SDK fixtures at two tick rates.
foreach(rate IN ITEMS 1000 100)
  set(wire_suite tusb32x_arduino_wire_${rate})
  add_executable(${wire_suite} arduino_wire/test_wire.cpp ../examples/arduino/Tusb32xWire.cpp)
  target_link_libraries(${wire_suite} PRIVATE TUSB32x)
  target_include_directories(${wire_suite} PRIVATE arduino_wire/fakes ../examples/arduino)
  target_compile_definitions(${wire_suite} PRIVATE CONFIG_DISABLE_HAL_LOCKS=1
    CORE_DEBUG_LEVEL=0 configTICK_RATE_HZ=${rate})
  if(rate EQUAL 1000)
    target_compile_definitions(${wire_suite} PRIVATE CONFIG_IDF_TARGET_ESP32S2=1 CONFIG_IDF_TARGET_ESP32S3=0)
  else()
    target_compile_definitions(${wire_suite} PRIVATE CONFIG_IDF_TARGET_ESP32S2=0 CONFIG_IDF_TARGET_ESP32S3=1)
  endif()
  tusb32x_strict_warnings(${wire_suite})
  if(MSVC)
    target_compile_options(${wire_suite} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${wire_suite} PRIVATE -UNDEBUG)
  endif()
  add_test(NAME ${wire_suite} COMMAND ${wire_suite})
  set_tests_properties(${wire_suite} PROPERTIES TIMEOUT 30)
endforeach()

# Record native SDK calls through the actual adapter at normal and coarse ticks.
foreach(rate IN ITEMS 1000 100)
  set(idf_suite tusb32x_idf_transport_${rate})
  add_executable(${idf_suite} idf_transport/test_idf.cpp ../examples/esp_idf/Tusb32xIdf.cpp)
  target_link_libraries(${idf_suite} PRIVATE TUSB32x)
  target_include_directories(${idf_suite} PRIVATE idf_transport/fakes ../examples/esp_idf)
  target_compile_definitions(${idf_suite} PRIVATE CONFIG_LOG_MAXIMUM_LEVEL=0
    CONFIG_PM_ENABLE=0 CONFIG_I2C_ENABLE_DEBUG_LOG=0 configTICK_RATE_HZ=${rate})
  if(rate EQUAL 1000)
    target_compile_definitions(${idf_suite} PRIVATE CONFIG_IDF_TARGET_ESP32S2=1 CONFIG_IDF_TARGET_ESP32S3=0)
  else()
    target_compile_definitions(${idf_suite} PRIVATE CONFIG_IDF_TARGET_ESP32S2=0 CONFIG_IDF_TARGET_ESP32S3=1)
  endif()
  tusb32x_strict_warnings(${idf_suite})
  if(MSVC)
    target_compile_options(${idf_suite} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${idf_suite} PRIVATE -UNDEBUG)
  endif()
  add_test(NAME ${idf_suite} COMMAND ${idf_suite})
  set_tests_properties(${idf_suite} PROPERTIES TIMEOUT 30)
endforeach()

# Exercise the actual Arduino application with recording SDK boundaries.
foreach(app IN ITEMS owner usb_sda usb_irq)
  set(app_suite tusb32x_arduino_app_${app})
  add_executable(${app_suite} arduino_app/test_app.cpp
    ../examples/arduino/Tusb32xWire.cpp ../examples/common/ExampleOwner.cpp)
  target_link_libraries(${app_suite} PRIVATE tusb32x_cli_support)
  target_include_directories(${app_suite} PRIVATE arduino_app/fakes
    arduino_wire/fakes ../examples/arduino)
  target_compile_definitions(${app_suite} PRIVATE TUSB32X_BOARD_READY=1
    TUSB32X_MAX_OVERHEAD_US=100 TUSB32X_ACCEPT_SDK_CLEANUP=1
    ARDUINO_RUNNING_CORE=0 ARDUINO_USB_MODE=1 CONFIG_DISABLE_HAL_LOCKS=1
    CORE_DEBUG_LEVEL=0 CONFIG_IDF_TARGET_ESP32S2=0 CONFIG_IDF_TARGET_ESP32S3=1)
  tusb32x_strict_warnings(${app_suite})
  if(MSVC)
    target_compile_options(${app_suite} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${app_suite} PRIVATE -UNDEBUG)
  endif()
  if(app STREQUAL "owner")
    foreach(scenario IN ITEMS startup startup-failure begin-failure preparation-failure
        cleanup-failure end-failure existing-bus shutdown)
      add_test(NAME tusb32x_arduino_app_${scenario} COMMAND ${app_suite} ${scenario})
      set_tests_properties(tusb32x_arduino_app_${scenario} PROPERTIES TIMEOUT 30)
    endforeach()
  else()
    if(app STREQUAL "usb_sda")
      target_compile_definitions(${app_suite} PRIVATE TUSB32X_SDA=19)
    else()
      target_compile_definitions(${app_suite} PRIVATE TUSB32X_IRQ=20)
    endif()
    add_test(NAME ${app_suite} COMMAND ${app_suite} usb-pins)
    set_tests_properties(${app_suite} PROPERTIES TIMEOUT 30)
  endif()
endforeach()

# Exercise the actual native application with recording SDK boundaries. Each
# scenario gets a fresh process because application-owned resources have one lifetime.
foreach(app IN ITEMS owner diagnostic pin_conflict irq runtime)
  set(app_suite tusb32x_idf_app_${app})
  add_executable(${app_suite} idf_app/test_app.cpp
    ../examples/esp_idf/Tusb32xIdf.cpp ../examples/common/ExampleOwner.cpp)
  target_link_libraries(${app_suite} PRIVATE tusb32x_cli_support)
  target_include_directories(${app_suite} PRIVATE idf_app/fakes
    idf_transport/fakes ../examples/esp_idf)
  target_compile_definitions(${app_suite} PRIVATE CONFIG_LOG_MAXIMUM_LEVEL=0
    CONFIG_PM_ENABLE=0 CONFIG_I2C_ENABLE_DEBUG_LOG=0 configTICK_RATE_HZ=1000
    CONFIG_IDF_TARGET_ESP32S2=0 CONFIG_IDF_TARGET_ESP32S3=1)
  if(NOT app STREQUAL "diagnostic")
    target_compile_definitions(${app_suite} PRIVATE TUSB32X_BOARD_READY=1
      TUSB32X_MAX_OVERHEAD_US=2000 TUSB32X_ACCEPT_SDK_CLEANUP=1)
  endif()
  tusb32x_strict_warnings(${app_suite})
  if(MSVC)
    target_compile_options(${app_suite} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${app_suite} PRIVATE -UNDEBUG)
  endif()
  if(app STREQUAL "owner")
    foreach(scenario IN ITEMS startup startup-failure shutdown-active shutdown-startup
        backpressure bus-failure device-failure uart-config-failure uart-pin-failure
        uart-install-failure console-already-owned device-remove-failure bus-delete-failure
        uart-delete-failure rx-overflow raw-error rx-overflow-after-read rx-event-flood
        device-cleanup-failure teardown-fifo-pending teardown-fifo-stalled existing-bus)
      add_test(NAME tusb32x_idf_app_${scenario} COMMAND ${app_suite} ${scenario})
      set_tests_properties(tusb32x_idf_app_${scenario} PROPERTIES TIMEOUT 30)
    endforeach()
  elseif(app STREQUAL "irq")
    target_compile_definitions(${app_suite} PRIVATE TUSB32X_IRQ=6)
    foreach(scenario IN ITEMS irq-configured irq-config-failure irq-cleanup-failure)
      add_test(NAME tusb32x_idf_app_${scenario} COMMAND ${app_suite} ${scenario})
      set_tests_properties(tusb32x_idf_app_${scenario} PROPERTIES TIMEOUT 30)
    endforeach()
  else()
    if(app STREQUAL "pin_conflict")
      target_compile_definitions(${app_suite} PRIVATE TUSB32X_SDA=43)
      set(scenario pin-conflict)
    elseif(app STREQUAL "runtime")
      target_compile_definitions(${app_suite} PRIVATE TUSB32X_RUN_MS=1000)
      set(scenario run-timeout)
    else()
      set(scenario diagnostic)
    endif()
    add_test(NAME ${app_suite} COMMAND ${app_suite} ${scenario})
    set_tests_properties(${app_suite} PROPERTIES TIMEOUT 30)
  endif()
endforeach()

# Each public core header is independently includable.
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
# No CTest entry: compiling signatures cannot prove driver behavior.

# Python is a maintainer test dependency only; tests-OFF consumers never find it.
find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
add_test(NAME tusb32x_package_safety
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_check_package.py" -v)
set_tests_properties(tusb32x_package_safety PROPERTIES TIMEOUT 60)
add_test(NAME tusb32x_package_consumer
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/check_package.py"
    --test-archive --out "${CMAKE_CURRENT_BINARY_DIR}/package-check"
    --cmake "${CMAKE_COMMAND}" --compiler "${CMAKE_CXX_COMPILER}"
    --make-program "${CMAKE_MAKE_PROGRAM}" --generator "${CMAKE_GENERATOR}"
    --config "$<IF:$<CONFIG:Debug>,Debug,Release>")
set_tests_properties(tusb32x_package_consumer PROPERTIES TIMEOUT 120)
add_test(NAME tusb32x_package_origins
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_check_package.py"
    --consumer-origins --cmake "${CMAKE_COMMAND}" --compiler "${CMAKE_CXX_COMPILER}"
    --make-program "${CMAKE_MAKE_PROGRAM}" --generator "${CMAKE_GENERATOR}"
    --config "$<IF:$<CONFIG:Debug>,Debug,Release>")
set_tests_properties(tusb32x_package_origins PROPERTIES TIMEOUT 120)
add_test(NAME tusb32x_idf_example_config
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_idf_example.py"
    --compiler "${CMAKE_CXX_COMPILER}" --cmake "${CMAKE_COMMAND}" --generator "${CMAKE_GENERATOR}")
set_tests_properties(tusb32x_idf_example_config PROPERTIES TIMEOUT 120)
add_test(NAME tusb32x_arduino_examples_config
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_arduino_examples.py"
    --compiler "${CMAKE_CXX_COMPILER}")
set_tests_properties(tusb32x_arduino_examples_config PROPERTIES TIMEOUT 60)
add_test(NAME cli_transcript
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_cli_transcript.py"
    --host "$<TARGET_FILE:tusb32x_cli_host>")
set_tests_properties(cli_transcript PROPERTIES TIMEOUT 30)
add_test(NAME hil_runner
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_tusb32x_hil_runner.py")
set_tests_properties(hil_runner PROPERTIES TIMEOUT 90)
add_test(NAME cli_contract
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/check_cli_contract.py"
    --host "$<TARGET_FILE:tusb32x_cli_host>")
add_test(NAME cli_contract_negative
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/check_cli_contract.py"
    --host "$<TARGET_FILE:tusb32x_cli_host>" --self-test-negative)
set_tests_properties(cli_contract cli_contract_negative PROPERTIES TIMEOUT 60)
# Imported objects must keep ABI/sanitizer flags in the downstream link.
set(consumer_flags "${CMAKE_CURRENT_BINARY_DIR}/consumer-flags.cmake")
file(WRITE "${consumer_flags}" "# Parent compile/link flags; no source or tool lookup.\n")
set(consumer_configurations Debug Release RelWithDebInfo MinSizeRel
  ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
list(REMOVE_DUPLICATES consumer_configurations)
set(consumer_flag_variables CMAKE_CXX_FLAGS CMAKE_EXE_LINKER_FLAGS)
foreach(configuration IN LISTS consumer_configurations)
  string(TOUPPER "${configuration}" configuration_upper)
  list(APPEND consumer_flag_variables
    CMAKE_CXX_FLAGS_${configuration_upper} CMAKE_EXE_LINKER_FLAGS_${configuration_upper})
endforeach()
foreach(variable IN LISTS consumer_flag_variables)
  file(APPEND "${consumer_flags}"
    "set(${variable} [==[${${variable}}]==] CACHE STRING \"Inherited consumer flags\" FORCE)\n")
endforeach()
add_test(NAME tusb32x_portable_consumer
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/consumer/run_portable.py"
    --source "${CMAKE_CURRENT_SOURCE_DIR}/consumer/portable"
    --include "${PROJECT_SOURCE_DIR}/include"
    --library "$<TARGET_FILE:TUSB32x>"
    --toolchain-flags "${consumer_flags}"
    --output "${CMAKE_CURRENT_BINARY_DIR}"
    --cmake "${CMAKE_COMMAND}"
    --compiler "${CMAKE_CXX_COMPILER}"
    --make-program "${CMAKE_MAKE_PROGRAM}"
    --generator "${CMAKE_GENERATOR}"
    --config "$<CONFIG>")
set_tests_properties(tusb32x_portable_consumer PROPERTIES TIMEOUT 120)
add_test(NAME tusb32x_version_sync
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_version_sync.py")
set_tests_properties(tusb32x_version_sync PROPERTIES TIMEOUT 120)

if(WIN32)
  add_test(NAME tusb32x_pio_launcher
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/test_pio_launcher.py")
  set_tests_properties(tusb32x_pio_launcher PROPERTIES TIMEOUT 30)
endif()
