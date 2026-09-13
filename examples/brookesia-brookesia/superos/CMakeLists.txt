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
    "${PUBLISHED_BROOKESIA_ROOT}/system/brookesia_system_core"
    "${CMAKE_BINARY_DIR}/components/brookesia_system_core")
add_subdirectory(
    "${PUBLISHED_BROOKESIA_ROOT}/system/brookesia_system_super"
    "${CMAKE_BINARY_DIR}/components/brookesia_system_super")

add_executable(bb_superos main.cpp)
target_link_libraries(bb_superos PRIVATE
    brookesia::system_super
    brookesia::gui_lvgl
    brookesia::hal_wasm)
target_link_options(bb_superos PRIVATE
    "SHELL:-sUSE_SDL=2"
    "SHELL:-sUSE_ZLIB=1"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sASYNCIFY=1"
    # SystemSuper stages this directory before the executable links.
    "SHELL:--preload-file ${CMAKE_BINARY_DIR}/brookesia@/brookesia")
set_target_properties(bb_superos PROPERTIES OUTPUT_NAME "bb-superos" SUFFIX ".html")
