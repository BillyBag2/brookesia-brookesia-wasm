cmake_minimum_required(VERSION 3.24)

project(brookesia_wasm LANGUAGES C CXX)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Configure this project with Emscripten (use emcmake cmake ...)")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(BROOKESIA_SOURCE_ROOT "${REPO_ROOT}/.deps/assembled/esp-brookesia")
set(LVGL_SOURCE_ROOT "${REPO_ROOT}/.deps/assembled/external/lvgl")

foreach(required_file
        "${BROOKESIA_SOURCE_ROOT}/gui/brookesia_gui_lvgl/CMakeLists.txt"
        "${BROOKESIA_SOURCE_ROOT}/hal/brookesia_hal_wasm/CMakeLists.txt"
        "${LVGL_SOURCE_ROOT}/CMakeLists.txt")
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR
            "Fetched sources are missing (${required_file}). Run fetch.ps1 before configuring.")
    endif()
endforeach()

# Keep the first browser target focused on the JSON layout and widget path.
set(LV_BUILD_CONF_PATH "${REPO_ROOT}/cmake/lv_conf.h" CACHE FILEPATH "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
add_subdirectory("${LVGL_SOURCE_ROOT}" "${CMAKE_BINARY_DIR}/components/lvgl")
target_include_directories(lvgl PUBLIC "${REPO_ROOT}/.deps/assembled/external")
include("${REPO_ROOT}/cmake/wasm_lvgl.cmake")
brookesia_configure_wasm_lvgl(lvgl)

# These component CMake files pull their own Brookesia dependencies from the
# assembled source tree. gui_lvgl also selects hal_wasm when EMSCRIPTEN is set.
add_subdirectory(
    "${BROOKESIA_SOURCE_ROOT}/utils/brookesia_lib_utils"
    "${CMAKE_BINARY_DIR}/components/brookesia_lib_utils")
# The released component enables Emscripten's Boost headers only at link time.
# The option is also required while compiling every consumer of its headers.
target_compile_options(brookesia_lib_utils_impl PUBLIC "SHELL:-sUSE_BOOST_HEADERS=1")
add_subdirectory(
    "${BROOKESIA_SOURCE_ROOT}/gui/brookesia_gui_interface"
    "${CMAKE_BINARY_DIR}/components/brookesia_gui_interface")
add_subdirectory(
    "${BROOKESIA_SOURCE_ROOT}/gui/brookesia_gui_lvgl"
    "${CMAKE_BINARY_DIR}/components/brookesia_gui_lvgl")
# The HAL has the same compile/link mismatch for Emscripten's SDL2 port.
target_compile_options(brookesia_hal_wasm_impl PUBLIC "SHELL:-sUSE_SDL=2")

add_executable(brookesia_layout_test main.cpp)
target_link_libraries(brookesia_layout_test PRIVATE
    brookesia::gui_lvgl
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,brookesia::gui_interface_examples>")
target_link_options(brookesia_layout_test PRIVATE
    "SHELL:-sUSE_SDL=2"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sASYNCIFY=1")
set_target_properties(brookesia_layout_test PROPERTIES OUTPUT_NAME "bb-layout" SUFFIX ".html")
