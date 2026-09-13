cmake_minimum_required(VERSION 3.24)
project(brookesia_superos_wasm LANGUAGES C CXX)
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Configure with Emscripten or run build-eb-superos.ps1")
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(BROOKESIA_SOURCE_ROOT "${REPO_ROOT}/thirdparty/esp-brookesia")
set(LVGL_SOURCE_ROOT "${REPO_ROOT}/.deps/assembled/external/lvgl")
if(NOT EXISTS "${BROOKESIA_SOURCE_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "Initialize the thirdparty/esp-brookesia submodule first")
endif()
if(NOT EXISTS "${LVGL_SOURCE_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "LVGL is missing. Run fetch.ps1 first")
endif()
# Upstream requires an existing LVGL target.
set(LV_BUILD_CONF_PATH "${REPO_ROOT}/cmake/lv_conf.h" CACHE FILEPATH "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
add_subdirectory("${LVGL_SOURCE_ROOT}" "${CMAKE_BINARY_DIR}/components/lvgl")
target_include_directories(lvgl PUBLIC "${REPO_ROOT}/.deps/assembled/external")
# Minimal SuperOS selection. Upstream owns component dependency resolution and
# stages the shell resources beneath ${CMAKE_BINARY_DIR}/brookesia.
foreach(component IN ITEMS
        LIB_UTILS HAL_INTERFACE HAL_WASM
        SERVICE_MANAGER SERVICE_HELPER SERVICE_STORAGE SERVICE_DEVICE SERVICE_WIFI SERVICE_DISPLAY
        GUI_INTERFACE GUI_LVGL RUNTIME_MANAGER SYSTEM_CORE SYSTEM_SUPER)
    set(BROOKESIA_BUILD_${component} ON)
endforeach()
add_subdirectory("${BROOKESIA_SOURCE_ROOT}" "${CMAKE_BINARY_DIR}/esp-brookesia")
# Emscripten ports must also be enabled while compiling consumers.
target_compile_options(brookesia_lib_utils_impl PUBLIC "SHELL:-sUSE_BOOST_HEADERS=1")
target_compile_options(brookesia_hal_wasm_impl PUBLIC "SHELL:-sUSE_SDL=2")

add_executable(eb_superos main.cpp)
target_link_libraries(eb_superos PRIVATE
    brookesia::system_super
    brookesia::gui_lvgl
    brookesia::hal_wasm
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,brookesia::service_device>")
target_link_options(eb_superos PRIVATE
    "SHELL:-sUSE_SDL=2"
    "SHELL:-sUSE_ZLIB=1"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sASYNCIFY=1"
    "SHELL:--preload-file ${CMAKE_BINARY_DIR}/brookesia@/brookesia")
set_target_properties(eb_superos PROPERTIES OUTPUT_NAME "eb-superos" SUFFIX ".html")

# Keep Emscripten's generated diagnostic shell and provide a cleaner static
# launcher beside it for normal use.
set(BROOKESIA_HOST_TITLE "ESP-Brookesia SuperOS")
set(BROOKESIA_HOST_SCRIPT "eb-superos.js")
configure_file(
    "${REPO_ROOT}/cmake/superos-host.html.in"
    "${CMAKE_CURRENT_BINARY_DIR}/eb-superos-host.html"
    @ONLY)

get_property(superos_resource_targets GLOBAL PROPERTY BROOKESIA_SYSTEM_SUPER_STAGE_TARGETS)
if(superos_resource_targets)
    add_dependencies(eb_superos ${superos_resource_targets})
endif()
