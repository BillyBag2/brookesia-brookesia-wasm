function(brookesia_configure_wasm_lvgl target)
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR "brookesia_configure_wasm_lvgl requires Emscripten")
    endif()

    # Features enabled by the shared lv_conf.h that require an Emscripten port
    # must be present while compiling LVGL and while linking its consumer.
    target_compile_options(${target} PUBLIC "SHELL:-sUSE_FREETYPE=1")
    target_link_options(${target} INTERFACE "SHELL:-sUSE_FREETYPE=1")
endfunction()
