#include <cstdlib>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>
#include <emscripten/eventloop.h>

#include "brookesia/gui_interface.hpp"
#include "brookesia/gui_lvgl/backend.hpp"
#include "brookesia/gui_lvgl/display_source.hpp"
#include "brookesia/hal_wasm/display/device.hpp"
#include "brookesia/service_display.hpp"
#include "brookesia/service_manager.hpp"
#include "brookesia/service_helper.hpp"
#include "brookesia/lib_utils/function_guard.hpp"

using namespace esp_brookesia;

namespace {

using DisplayHelper = service::helper::Display;
// Retain the service binding after main returns to the browser event loop.
service::ServiceBinding display_binding;
std::unique_ptr<gui::Runtime> runtime;
std::unique_ptr<gui::examples::ExampleRunner> runner;
int runner_interval = 0;

void process_example_loop(void *)
{
    try {
        gui::lvgl::lock_thread();
        lib_utils::FunctionGuard unlock_guard(gui::lvgl::unlock_thread);
        runtime->process_backend();
        runner->process_pending();
    } catch (const std::exception &error) {
        emscripten_clear_interval(runner_interval);
        std::cerr << "Example processing failed: " << error.what() << '\n';
    }
}

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

    display_binding = service::ServiceManager::get_instance().bind(DisplayHelper::get_name().data());
    if (!display_binding.is_valid()) {
        std::cerr << "Could not bind the display service\n";
        return EXIT_FAILURE;
    }

    auto outputs_result = DisplayHelper::call_function_sync<boost::json::array>(
        DisplayHelper::FunctionId::GetOutputs, service::helper::Timeout(5000));
    if (!outputs_result) {
        std::cerr << "Could not get display outputs: " << outputs_result.error() << '\n';
        return EXIT_FAILURE;
    }
    std::vector<DisplayHelper::OutputInfo> outputs;
    if (!BROOKESIA_DESCRIBE_FROM_JSON(outputs_result.value(), outputs)) {
        std::cerr << "Could not parse display outputs\n";
        return EXIT_FAILURE;
    }
    for (const auto &output : outputs) {
        if (!output.backlight.has_value()) {
            continue;
        }
        auto light_result = DisplayHelper::call_function_sync(
            DisplayHelper::FunctionId::SetBacklightOnOff,
            static_cast<double>(output.id), true, service::helper::Timeout(5000));
        if (!light_result) {
            std::cerr << "Could not turn on display backlight: " << light_result.error() << '\n';
            return EXIT_FAILURE;
        }
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
    lib_utils::FunctionGuard unlock_guard(gui::lvgl::unlock_thread);
    auto result = runner->start();
    if (result) {
        runtime->process_backend();
    }
    if (!result) {
        std::cerr << "Could not mount the Brookesia example menu: " << result.error() << '\n';
        return EXIT_FAILURE;
    }

    // DisplaySource owns the Emscripten main loop. Process navigation separately,
    // outside LVGL event dispatch, without replacing that display loop.
    runner_interval = emscripten_set_interval(process_example_loop, 16.0, nullptr);
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
