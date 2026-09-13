add_library(brookesia_board_m5stack_tab5_wasm STATIC
    src/m5stack_tab5.cpp)

add_library(brookesia::board_m5stack_tab5_wasm ALIAS
    brookesia_board_m5stack_tab5_wasm)
add_library(brookesia::board_wasm ALIAS
    brookesia_board_m5stack_tab5_wasm)

target_include_directories(brookesia_board_m5stack_tab5_wasm PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/../../../brookesia-brookesia/boards/m5stack_tab5/common/include")

target_compile_features(brookesia_board_m5stack_tab5_wasm PUBLIC cxx_std_23)
