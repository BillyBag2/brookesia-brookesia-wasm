cmake_minimum_required(VERSION 3.24)
project(brookesia_brookesia_superos_wasm LANGUAGES C CXX)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Configure with Emscripten")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(PUBLISHED_BROOKESIA_ROOT "${REPO_ROOT}/.deps/assembled/esp-brookesia")
set(LVGL_SOURCE_ROOT "${REPO_ROOT}/.deps/assembled/external/lvgl")

set(BROOKESIA_BOARD "m5stack_tab5" CACHE STRING
    "Board implementation used by the Brookesia WASM target")
set_property(CACHE BROOKESIA_BOARD PROPERTY STRINGS m5stack_tab5)
set(BROOKESIA_BOARD_SOURCE_DIR "${REPO_ROOT}/boards/${BROOKESIA_BOARD}/wasm")

if(NOT EXISTS "${BROOKESIA_BOARD_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "Unsupported BROOKESIA_BOARD='${BROOKESIA_BOARD}'. Expected a WASM board component at "
        "${BROOKESIA_BOARD_SOURCE_DIR}/CMakeLists.txt")
endif()

message(STATUS "Brookesia WASM board: ${BROOKESIA_BOARD}")

foreach(required_path
        "${PUBLISHED_BROOKESIA_ROOT}/system/brookesia_system_super/CMakeLists.txt"
        "${PUBLISHED_BROOKESIA_ROOT}/hal/brookesia_hal_wasm/CMakeLists.txt"
        "${LVGL_SOURCE_ROOT}/CMakeLists.txt")
    if(NOT EXISTS "${required_path}")
        message(FATAL_ERROR "Missing published dependency: ${required_path}. Run fetch.ps1.")
    endif()
endforeach()

set(LV_BUILD_CONF_PATH "${REPO_ROOT}/cmake/lv_conf.h" CACHE FILEPATH "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
add_subdirectory("${LVGL_SOURCE_ROOT}" "${CMAKE_BINARY_DIR}/components/lvgl")
target_include_directories(lvgl PUBLIC "${REPO_ROOT}/.deps/assembled/external")
include("${REPO_ROOT}/cmake/wasm_lvgl.cmake")
brookesia_configure_wasm_lvgl(lvgl)

# Route the build through fetched component sources. The component CMake files
# add their published dependencies as required. HAL_WASM is currently consumed
# from the assembled source tree because no separately consumable WASM HAL
# package is available.
add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/utils/brookesia_lib_utils"
    "${CMAKE_BINARY_DIR}/components/brookesia_lib_utils")
target_compile_options(brookesia_lib_utils_impl PUBLIC "SHELL:-sUSE_BOOST_HEADERS=1")
add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/hal/brookesia_hal_wasm"
    "${CMAKE_BINARY_DIR}/components/brookesia_hal_wasm")
target_compile_options(brookesia_hal_wasm_impl PUBLIC "SHELL:-sUSE_SDL=2")
add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/gui/brookesia_gui_interface"
    "${CMAKE_BINARY_DIR}/components/brookesia_gui_interface")
add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/gui/brookesia_gui_lvgl"
    "${CMAKE_BINARY_DIR}/components/brookesia_gui_lvgl")
add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/runtime/brookesia_runtime_manager"
    "${CMAKE_BINARY_DIR}/components/brookesia_runtime_manager")
add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/service/system/brookesia_service_device"
    "${CMAKE_BINARY_DIR}/components/brookesia_service_device")

# The WASM storage HAL exposes this build directory as /brookesia/fs/littlefs.
# Stage registered app packages inside that volume so SystemCore can install
# them from the same path it uses on the native target.
set(
    BROOKESIA_SYSTEM_CORE_PC_STAGE_APP_ROOT
    "${CMAKE_BINARY_DIR}/brookesia/fs/littlefs/apps"
    CACHE PATH
    "WASM staging root for registered apps"
    FORCE)
# Remove packages left by builds made before apps were staged inside LittleFS.
file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/brookesia/apps")
add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/system/brookesia_system_core"
    "${CMAKE_BINARY_DIR}/components/brookesia_system_core")

set(BROOKESIA_WASM_APP_MANIFEST "${BROOKESIA_BOARD_SOURCE_DIR}/apps.cmake")
if(NOT EXISTS "${BROOKESIA_WASM_APP_MANIFEST}")
    message(FATAL_ERROR
        "Board '${BROOKESIA_BOARD}' has no WASM app manifest: ${BROOKESIA_WASM_APP_MANIFEST}")
endif()
include("${BROOKESIA_WASM_APP_MANIFEST}")

set(BROOKESIA_WASM_APP_TARGETS "")
foreach(app_name IN LISTS BROOKESIA_WASM_APPS)
    if(NOT app_name MATCHES "^brookesia_app_[a-z0-9_]+$")
        message(FATAL_ERROR "Invalid Brookesia WASM app name: '${app_name}'")
    endif()

    set(app_source_dir "${PUBLISHED_BROOKESIA_ROOT}/app/${app_name}")
    if(NOT EXISTS "${app_source_dir}/CMakeLists.txt")
        message(FATAL_ERROR
            "WASM app '${app_name}' is selected for '${BROOKESIA_BOARD}' but is missing from "
            "${app_source_dir}. Add its pinned entry to wasm-components.lock.json and run fetch.ps1.")
    endif()

    add_subdirectory(
        "${app_source_dir}"
        "${CMAKE_BINARY_DIR}/components/${app_name}")
    string(REGEX REPLACE "^brookesia_" "" app_target_name "${app_name}")
    list(APPEND BROOKESIA_WASM_APP_TARGETS "brookesia::${app_target_name}")
    message(STATUS "Brookesia WASM app: ${app_name}")
endforeach()

add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/system/brookesia_system_super"
    "${CMAKE_BINARY_DIR}/components/brookesia_system_super")
add_subdirectory(
    "${BROOKESIA_BOARD_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}/components/board_${BROOKESIA_BOARD}")

add_executable(bb_superos main.cpp)
target_link_libraries(bb_superos PRIVATE
    brookesia::board_wasm
    brookesia::system_super
    brookesia::gui_lvgl
    brookesia::hal_wasm
    ${BROOKESIA_WASM_APP_TARGETS}
    brookesia::service_device)
if(BROOKESIA_WASM_APP_TARGETS)
    target_compile_definitions(bb_superos PRIVATE BROOKESIA_WASM_HAS_REGISTERED_APPS=1)
else()
    target_compile_definitions(bb_superos PRIVATE BROOKESIA_WASM_HAS_REGISTERED_APPS=0)
endif()
target_link_options(bb_superos PRIVATE
    "SHELL:-sUSE_SDL=2"
    "SHELL:-sUSE_ZLIB=1"
    "SHELL:-sSTACK_SIZE=262144"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sASYNCIFY=1"
    # SystemSuper stages this directory before the executable links.
    "SHELL:--preload-file ${CMAKE_BINARY_DIR}/brookesia@/brookesia")
file(GLOB_RECURSE brookesia_preload_files CONFIGURE_DEPENDS
    LIST_DIRECTORIES FALSE
    "${CMAKE_BINARY_DIR}/brookesia/*")
set_property(TARGET bb_superos APPEND PROPERTY LINK_DEPENDS ${brookesia_preload_files})
set_target_properties(bb_superos PROPERTIES OUTPUT_NAME "bb-superos" SUFFIX ".html")

get_property(superos_resource_targets GLOBAL PROPERTY BROOKESIA_SYSTEM_SUPER_STAGE_TARGETS)
get_property(runtime_app_targets GLOBAL PROPERTY BROOKESIA_RUNTIME_APP_STAGE_TARGETS)
if(superos_resource_targets OR runtime_app_targets)
    add_dependencies(bb_superos ${superos_resource_targets} ${runtime_app_targets})
endif()

# Apply the firmware project's sparse overrides after the published components
# stage their baseline packages. Native and WASM builds use the same helper and
# source tree while retaining independent generated staging directories.
set(BROOKESIA_FIRMWARE_ROOT "${REPO_ROOT}/brookesia-brookesia")
include("${BROOKESIA_FIRMWARE_ROOT}/cmake/resource_overrides.cmake")
brookesia_add_resource_overrides_target(bb_superos_apply_resource_overrides
    SOURCE_DIR "${BROOKESIA_FIRMWARE_ROOT}/resource_overrides"
    SYSTEM_ROOT "${CMAKE_BINARY_DIR}/brookesia"
    LITTLEFS_ROOT "${CMAKE_BINARY_DIR}/brookesia/fs/littlefs"
    DEPENDS ${superos_resource_targets} ${runtime_app_targets})
add_dependencies(bb_superos bb_superos_apply_resource_overrides)
set_property(TARGET bb_superos APPEND PROPERTY LINK_DEPENDS
    ${bb_superos_apply_resource_overrides_FILES})

# Keep Emscripten's generated diagnostic shell and provide a cleaner static
# launcher beside it for normal use.
set(BROOKESIA_HOST_TITLE "Brookesia-Brookesia SuperOS")
set(BROOKESIA_HOST_SCRIPT "bb-superos.js")
configure_file(
    "${REPO_ROOT}/cmake/superos-host.html.in"
    "${CMAKE_CURRENT_BINARY_DIR}/bb-superos-host.html"
    @ONLY)
