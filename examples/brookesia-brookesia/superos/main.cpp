#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

#include "boost/json/array.hpp"
#include <emscripten.h>
#include "brookesia/board/config.hpp"
#include "brookesia/board_wasm/config.hpp"
#include "brookesia/gui_lvgl/backend.hpp"
#include "brookesia/gui_lvgl/display_source.hpp"
#include "brookesia/hal_wasm/display/device.hpp"
#include "brookesia/service_helper.hpp"
#include "brookesia/service_manager.hpp"
#include "brookesia/system_super.hpp"

using namespace esp_brookesia;

namespace {

using DisplayHelper = service::helper::Display;

// These objects must outlive main because Emscripten returns to the browser
// event loop before the deferred SuperOS startup runs.
service::ServiceBinding display_binding;
std::unique_ptr<system::super::System> system_instance;

bool enable_backlights()
{
    auto outputs = DisplayHelper::call_function_sync<boost::json::array>(
        DisplayHelper::FunctionId::GetOutputs, service::helper::Timeout(5000));
    if (!outputs) {
        std::cerr << "Could not get display outputs: " << outputs.error() << '\n';
        return false;
    }

    std::vector<DisplayHelper::OutputInfo> output_info;
    if (!BROOKESIA_DESCRIBE_FROM_JSON(outputs.value(), output_info)) {
        std::cerr << "Could not parse display outputs\n";
        return false;
    }
    for (const auto &output : output_info) {
        if (!output.backlight.has_value()) {
            continue;
        }
        auto result = DisplayHelper::call_function_sync(
            DisplayHelper::FunctionId::SetBacklightOnOff, static_cast<double>(output.id), true,
            service::helper::Timeout(5000));
        if (!result) {
            std::cerr << "Could not turn on display backlight: " << result.error() << '\n';
            return false;
        }
    }
    return true;
}

void start_superos(void *)
{
    try {
        auto &source = gui::lvgl::DisplaySource::get_instance();
        constexpr auto appearance = board::appearance_config();
        system::super::System::Config config;
        config.core_config.gui_backend = std::make_unique<gui::lvgl::Backend>();
        config.core_config.environment = {
            .width_px = static_cast<int32_t>(source.width()),
            .height_px = static_cast<int32_t>(source.height()),
            .density = appearance.density,
            .font_scale = appearance.font_scale,
            .language = "en",
            .theme_id = "default",
        };
        config.core_config.start_service_manager = false;
        config.core_config.install_registered_apps = false;
        config.core_config.install_package_apps = false;
        config.resource_root_path = "/brookesia";
        config.core_config.storage.internal_override = system::core::StorageVolume{
            .id = "wasm_internal",
            .partition = system::core::StoragePartition::Internal,
            .mount_point = "/littlefs",
            .root_path = "/brookesia/fs/littlefs",
            .available = true,
        };

        system_instance = std::make_unique<system::super::System>();
        if (auto result = system_instance->init(std::move(config)); !result) {
            std::cerr << "Could not initialize SuperOS: " << result.error() << '\n';
            return;
        }
        if (auto result = system_instance->start(); !result) {
            std::cerr << "Could not start SuperOS: " << result.error() << '\n';
        }
    } catch (const std::exception &error) {
        std::cerr << "SuperOS startup failed: " << error.what() << '\n';
    }
}

} // namespace

int main()
{
    constexpr auto display_config = brookesia::board_wasm::native_display_config();
    auto &display = hal::DisplayWasmDevice::get_instance();
    if (!display.configure({
            .width_px = display_config.width,
            .height_px = display_config.height,
            .window_title = "Brookesia SuperOS",
        })) {
        return EXIT_FAILURE;
    }
    if (!service::ServiceManager::get_instance().start()) {
        return EXIT_FAILURE;
    }

    display_binding = service::ServiceManager::get_instance().bind(DisplayHelper::get_name().data());
    if (!display_binding.is_valid() || !enable_backlights()) {
        return EXIT_FAILURE;
    }

    auto &source = gui::lvgl::DisplaySource::get_instance();
    gui::lvgl::DisplaySourceConfig source_config;
    if (!source.start(source_config)) {
        return EXIT_FAILURE;
    }
    auto activated = DisplayHelper::call_function_sync(
                         DisplayHelper::FunctionId::SetActiveSourceRole, source.output_name(),
                         source_config.source_role, service::helper::Timeout(5000));
    if (!activated) {
        std::cerr << "Could not activate the LVGL display source: " << activated.error() << '\n';
        return EXIT_FAILURE;
    }

    emscripten_async_call(start_superos, nullptr, 0);
    return EXIT_SUCCESS;
}
