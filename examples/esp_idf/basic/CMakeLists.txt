# Native application: the root IDF component continues to register only the core.
cmake_minimum_required(VERSION 3.16)
get_filename_component(TUSB32X_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(EXTRA_COMPONENT_DIRS "${TUSB32X_ROOT}")

# A build directory owns its sdkconfig even without an explicit -DSDKCONFIG.
# Separate -B directories therefore cannot silently retarget each other's file.
if(NOT DEFINED SDKCONFIG)
  set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig" CACHE FILEPATH "Application configuration")
endif()
get_filename_component(SDKCONFIG "${SDKCONFIG}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")
set(SDKCONFIG "${SDKCONFIG}" CACHE FILEPATH "Application configuration" FORCE)
# Keep app metadata aligned with library version truth, including archive builds.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${TUSB32X_ROOT}/library.json")
file(READ "${TUSB32X_ROOT}/library.json" TUSB32X_MANIFEST)
string(REGEX MATCH "\"version\"[ \t\r\n]*:[ \t\r\n]*\"([^\"]+)\""
  TUSB32X_VERSION_MATCH "${TUSB32X_MANIFEST}")
set(PROJECT_VER "${CMAKE_MATCH_1}")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# The component manager records the target too; keep its generated lock beside
# sdkconfig instead of letting concurrent targets overwrite a source-tree file.
idf_build_set_property(DEPENDENCIES_LOCK "${CMAKE_BINARY_DIR}/dependencies.lock")
project(tusb32x_idf_cli)
