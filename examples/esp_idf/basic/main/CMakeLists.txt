# IDF names an extra component after its directory, including renamed archives.
get_filename_component(TUSB32X_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
get_filename_component(TUSB32X_COMPONENT_NAME "${TUSB32X_ROOT}" NAME)
idf_component_register(
  SRCS "main.cpp" "../../ExampleApp.cpp" "../../Tusb32xIdf.cpp"
    "../../../common/Tusb32xCli.cpp" "../../../common/Tusb32xCliOutput.cpp"
    "../../../common/ExampleOwner.cpp"
  INCLUDE_DIRS "." "../.." "../../../common"
  REQUIRES ${TUSB32X_COMPONENT_NAME} esp_driver_i2c esp_driver_gpio esp_driver_uart
    esp_timer esp_system freertos
)
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)

# BoardConfig supplies safe diagnostic defaults; -D cache overrides are explicit
# application choices and apply to every source in this application component.
foreach(TUSB32X_SETTING VARIANT ADDRESS SDA SCL IRQ ENABLE
    BUS_HZ CALLBACK_MS OPERATION_MS BOARD_READY MAX_OVERHEAD_US ACCEPT_SDK_CLEANUP
    RUN_MS UART_TX UART_RX)
  if(DEFINED TUSB32X_${TUSB32X_SETTING})
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
      "TUSB32X_${TUSB32X_SETTING}=${TUSB32X_${TUSB32X_SETTING}}")
  endif()
endforeach()

# Run on every build, rather than only CMake reconfiguration. Git/dirty/time
# evidence must follow an incremental source rebuild as well as a clean build.
set(TUSB32X_IDENTITY_DIR "${CMAKE_CURRENT_BINARY_DIR}/example_identity")
add_custom_target(tusb32x_idf_identity
  COMMAND "${PYTHON}" "${TUSB32X_ROOT}/scripts/idf_identity.py"
    --root "${TUSB32X_ROOT}" --output "${TUSB32X_IDENTITY_DIR}/FirmwareBuild.h"
    --target "${IDF_TARGET}"
  BYPRODUCTS "${TUSB32X_IDENTITY_DIR}/FirmwareBuild.h"
  VERBATIM
)
add_dependencies(${COMPONENT_LIB} tusb32x_idf_identity)
target_include_directories(${COMPONENT_LIB} PRIVATE "${TUSB32X_IDENTITY_DIR}")
