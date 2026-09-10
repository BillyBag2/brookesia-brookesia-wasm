#include <cstdlib>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>

#include "brookesia/gui_interface.hpp"
#include "brookesia/gui_lvgl/backend.hpp"
#include "brookesia/gui_lvgl/display_source.hpp"
#include "brookesia/hal_wasm/display/device.hpp"
#include "brookesia/service_manager.hpp"

using namespace esp_brookesia;

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
    if (!display_source.start({})) {
        std::cerr << "Could not start the LVGL display source\n";
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
    auto result = runner->start();
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
