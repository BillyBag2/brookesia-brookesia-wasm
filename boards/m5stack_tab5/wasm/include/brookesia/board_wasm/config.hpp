#pragma once

#include <cstdint>

#include "brookesia/board/config.hpp"

namespace brookesia::board_wasm {

enum class PixelFormat {
    rgb565,
};

struct DisplayConfig {
    std::uint16_t width;
    std::uint16_t height;
    PixelFormat pixel_format;
};

[[nodiscard]] constexpr DisplayConfig native_display_config() noexcept
{
    return {
        .width = 720,
        .height = 1280,
        .pixel_format = PixelFormat::rgb565,
    };
}

[[nodiscard]] constexpr esp_brookesia::board::AppearanceConfig appearance_config() noexcept
{
    auto config = esp_brookesia::board::appearance_config();
    config.density = 1.0F;
    return config;
}

} // namespace brookesia::board_wasm
