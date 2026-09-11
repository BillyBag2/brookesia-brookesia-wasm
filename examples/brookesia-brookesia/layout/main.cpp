#include <cstdlib>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

#include "brookesia/gui_interface.hpp"
#include "brookesia/gui_lvgl/backend.hpp"
#include "brookesia/gui_lvgl/display_source.hpp"
#include "brookesia/hal_wasm/display/device.hpp"
#include "brookesia/service_display.hpp"
#include "brookesia/service_manager.hpp"
#include "lvgl/lvgl.h"

using namespace esp_brookesia;

extern "C" int brookesia_wasm_display_snapshot_width();
extern "C" int brookesia_wasm_display_snapshot_height();
extern "C" int brookesia_wasm_display_snapshot_copy_rgba(uint8_t *destination, int destination_size);

namespace {

std::unique_ptr<gui::Runtime> runtime;
std::unique_ptr<gui::examples::ExampleRunner> runner;

int start_layout_test()
{
    auto &display_device = hal::DisplayWasmDevice::get_instance();
    if (!display_device.configure({
            .width_px = 800,
            .height_px = 480,
            .window_title = "Brookesia layout test",
        })) {
        std::cerr << "Could not configure the WASM display\n";
        return EXIT_FAILURE;
    }

    if (!service::ServiceManager::get_instance().start()) {
        std::cerr << "Could not start the Brookesia service manager\n";
        return EXIT_FAILURE;
    }

    auto &display_source = gui::lvgl::DisplaySource::get_instance();
    gui::lvgl::DisplaySourceConfig display_source_config;
    if (!display_source.start(display_source_config)) {
        std::cerr << "Could not start the LVGL display source\n";
        return EXIT_FAILURE;
    }

    auto activated = service::Display::get_instance().set_active_source_role(
                         display_source.output_name(), display_source_config.source_role);
    if (!activated) {
        std::cerr << "Could not activate the LVGL display source: " << activated.error() << '\n';
        return EXIT_FAILURE;
    }

    gui::Environment environment{
        .width_px = static_cast<int32_t>(display_source.width()),
        .height_px = static_cast<int32_t>(display_source.height()),
        .density = 1.0F,
        .font_scale = 1.0F,
        .language = "en-GB",
        .theme_id = "default",
    };

    runtime = std::make_unique<gui::Runtime>(std::make_unique<gui::lvgl::Backend>());
    runner = std::make_unique<gui::examples::ExampleRunner>(*runtime, environment);
    if (runner->example_count() == 0) {
        std::cerr << "No Brookesia GUI examples were linked into the browser build\n";
        return EXIT_FAILURE;
    }

    gui::lvgl::lock_thread();
    auto result = runner->start();
    if (result) {
        runtime->process_backend();

        auto *diagnostic = lv_label_create(lv_layer_top());
        lv_label_set_text(diagnostic, "Brookesia WASM - 40 GUI examples loaded");
        lv_obj_set_style_bg_color(diagnostic, lv_color_hex(0xDC2626), 0);
        lv_obj_set_style_bg_opa(diagnostic, LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(diagnostic, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_pad_all(diagnostic, 8, 0);
        lv_obj_align(diagnostic, LV_ALIGN_TOP_MID, 0, 8);

        lv_obj_invalidate(lv_screen_active());
        lv_obj_invalidate(lv_layer_top());
        lv_refr_now(display_source.display());

        const int snapshot_width = brookesia_wasm_display_snapshot_width();
        const int snapshot_height = brookesia_wasm_display_snapshot_height();
        std::vector<uint8_t> snapshot(
            static_cast<size_t>(snapshot_width) * static_cast<size_t>(snapshot_height) * 4U);
        const int snapshot_bytes = brookesia_wasm_display_snapshot_copy_rgba(
                                       snapshot.data(), static_cast<int>(snapshot.size()));
        size_t non_black_pixels = 0;
        for (size_t offset = 0; offset + 3 < static_cast<size_t>(snapshot_bytes); offset += 4) {
            if (snapshot[offset] != 0 || snapshot[offset + 1] != 0 || snapshot[offset + 2] != 0) {
                ++non_black_pixels;
            }
        }
        std::cout << "Initial LVGL frame: " << snapshot_width << 'x' << snapshot_height
                  << ", bytes=" << snapshot_bytes
                  << ", non-black pixels=" << non_black_pixels << '\n';
    }
    gui::lvgl::unlock_thread();
    if (!result) {
        std::cerr << "Could not mount the Brookesia example menu: " << result.error() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

} // namespace

int main()
{
    try {
        return start_layout_test();
    } catch (const std::exception &error) {
        std::cerr << "Brookesia layout test failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
