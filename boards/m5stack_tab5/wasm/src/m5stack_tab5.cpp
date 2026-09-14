#include "brookesia/board_wasm/config.hpp"

namespace brookesia::board_wasm {

static_assert(native_display_config().width == 720);
static_assert(native_display_config().height == 1280);
static_assert(native_display_config().pixel_format == PixelFormat::rgb565);
static_assert(appearance_config().density == 1.0F);
static_assert(appearance_config().font_scale == 1.0F);

} // namespace brookesia::board_wasm
