#include <cstdlib>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

#include <emscripten.h>
#include "boost/json/array.hpp"
#include "brookesia/gui_lvgl/backend.hpp"
#include "brookesia/gui_lvgl/display_source.hpp"
#include "brookesia/hal_wasm/display/device.hpp"
#include "brookesia/service_helper.hpp"
#include "brookesia/service_manager.hpp"
#include "brookesia/system_super.hpp"

using namespace esp_brookesia;

namespace {

using DisplayHelper = service::helper::Display;

// Objects must outlive main because Emscripten resumes execution through the
// display source's browser callback.
service::ServiceBinding display_binding;
std::unique_ptr<system::super::System> system_instance;

bool enable_backlights()
{
    auto outputs_result = DisplayHelper::call_function_sync<boost::json::array>(
        DisplayHelper::FunctionId::GetOutputs, service::helper::Timeout(5000));
    if (!outputs_result) {
        std::cerr << "Could not get display outputs: " << outputs_result.error() << '\n';
        return false;
    }

    std::vector<DisplayHelper::OutputInfo> outputs;
    if (!BROOKESIA_DESCRIBE_FROM_JSON(outputs_result.value(), outputs)) {
        std::cerr << "Could not parse display outputs\n";
        return false;
    }

    for (const auto &output : outputs) {
        if (!output.backlight.has_value()) {
            continue;
        }
        auto result = DisplayHelper::call_function_sync(
            DisplayHelper::FunctionId::SetBacklightOnOff,
            static_cast<double>(output.id), true, service::helper::Timeout(5000));
        if (!result) {
            std::cerr << "Could not turn on display backlight: " << result.error() << '\n';
            return false;
        }
    }
    return true;
}

void start_system_after_browser_loop(void *)
{
    try {
        auto &display_source = gui::lvgl::DisplaySource::get_instance();
        std::cout << "Initializing SuperOS after the browser event loop starts\n";

        system::super::System::Config config;
        // SuperOS normally stores packaged resources beneath internal storage.
        // In WASM, resources are preloaded at /brookesia while LittleFS is writable.
        config.resource_root_path = "/brookesia";
        config.core_config.gui_backend = std::make_unique<gui::lvgl::Backend>();
        config.core_config.environment = {
            .width_px = static_cast<int32_t>(display_source.width()),
            .height_px = static_cast<int32_t>(display_source.height()),
            .density = 1.0F,
            .font_scale = 1.0F,
            .language = "en",
            .theme_id = "default",
        };
        config.core_config.start_service_manager = false;
        config.core_config.install_registered_apps = false;
        config.core_config.install_package_apps = false;
        config.core_config.storage.internal_override = system::core::StorageVolume{
            .id = "wasm_internal",
            .partition = system::core::StoragePartition::Internal,
            // Keep writable system data separate from the preloaded /brookesia
            // resource archive. StorageWasmDevice exposes this LittleFS volume.
            .mount_point = "/littlefs",
            .root_path = "/brookesia/fs/littlefs",
            .available = true,
        };

        system_instance = std::make_unique<system::super::System>();
        auto initialized = system_instance->init(std::move(config));
        if (!initialized) {
            std::cerr << "Could not initialize SuperOS: " << initialized.error() << '\n';
            return;
        }
        std::cout << "SuperOS initialized; starting shell\n";
        auto started = system_instance->start();
        if (!started) {
            std::cerr << "Could not start SuperOS: " << started.error() << '\n';
            return;
        }
        std::cout << "SuperOS shell started\n";
    } catch (const std::exception &error) {
        std::cerr << "SuperOS startup failed: " << error.what() << '\n';
    }
}

int prepare_superos()
{
    auto &display_device = hal::DisplayWasmDevice::get_instance();
    if (!display_device.configure({
            .width_px = 800,
            .height_px = 480,
            .window_title = "Brookesia SuperOS",
        })) {
        std::cerr << "Could not configure the WASM display\n";
        return EXIT_FAILURE;
    }

    auto &service_manager = service::ServiceManager::get_instance();
    if (!service_manager.start()) {
        std::cerr << "Could not start the Brookesia service manager\n";
        return EXIT_FAILURE;
    }

    display_binding = service_manager.bind(DisplayHelper::get_name().data());
    if (!display_binding.is_valid()) {
        std::cerr << "Could not bind the display service\n";
        return EXIT_FAILURE;
    }
    if (!enable_backlights()) {
        return EXIT_FAILURE;
    }

    auto &display_source = gui::lvgl::DisplaySource::get_instance();
    gui::lvgl::DisplaySourceConfig display_source_config;
    if (!display_source.start(display_source_config)) {
        std::cerr << "Could not start the LVGL display source\n";
        return EXIT_FAILURE;
    }
    auto activated = DisplayHelper::call_function_sync(
                         DisplayHelper::FunctionId::SetActiveSourceRole,
                         display_source.output_name(), display_source_config.source_role,
                         service::helper::Timeout(5000));
    if (!activated) {
        std::cerr << "Could not activate the LVGL display source: " << activated.error() << '\n';
        return EXIT_FAILURE;
    }

    // DisplaySource owns the persistent Emscripten loop. Scheduling this after
    // main returns lets SystemCore's WASM single-thread scheduler use browser
    // callbacks during its startup work.
    emscripten_async_call(start_system_after_browser_loop, nullptr, 0);
    return EXIT_SUCCESS;
}

} // namespace

int main()
{
    try {
        return prepare_superos();
    } catch (const std::exception &error) {
        std::cerr << "SuperOS startup failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
