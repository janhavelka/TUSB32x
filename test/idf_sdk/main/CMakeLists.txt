# Resolve the registered component name from the checkout, including renamed archives.
get_filename_component(TUSB32X_COMPONENT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
get_filename_component(TUSB32X_COMPONENT_NAME "${TUSB32X_COMPONENT_DIR}" NAME)
idf_component_register(
  SRCS "main.cpp" "../../../examples/esp_idf/Tusb32xIdf.cpp"
  INCLUDE_DIRS "../../../examples/esp_idf"
  REQUIRES ${TUSB32X_COMPONENT_NAME} esp_driver_i2c esp_timer freertos
)
target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)
