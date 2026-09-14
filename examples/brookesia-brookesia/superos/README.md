# Brookesia-Brookesia SuperOS WASM experiment

This is the published-component counterpart to `examples/esp-brookesia/superos`.
It sources SuperOS, System Core, GUI, services, and LVGL from `.deps/assembled`.
The fetch lock obtains `brookesia_hal_wasm` and `brookesia_system_super` from a
fixed commit of the `BillyBag2/esp-brookesia` fork. The remaining Brookesia
components come from fixed ESP Component Registry releases.

## Current route

`CMakeLists.txt` adds the selected component projects directly and builds
`bb-superos.html` plus the clean `bb-superos-host.html` launcher. The former is
Emscripten's generated diagnostic page; the latter is the normal host page and
contains only the application canvas and loading/error status. `BROOKESIA_BOARD`
selects an ESP-IDF-free implementation under
`boards/<board>/wasm` and defaults to `m5stack_tab5`. The build wrapper stores the
result in `build/bb-superos/<board>` so CMake settings for different boards cannot
collide. Every board publishes the `brookesia::board_wasm` target and provides the
stable `brookesia/board_wasm/config.hpp` API. The launcher reads its display
dimensions from that API; for TAB5 it configures the WASM display, touch surface,
LVGL output, and SuperOS environment as 720 x 1280 portrait.
Portable appearance settings such as density and font scale are compiled directly
from `brookesia-brookesia/boards/<board>/common`, rather than `.deps`, so the
native firmware and browser build use the same board profile.

The selected board's `wasm/apps.cmake` manually allowlists registered applications.
TAB5 currently enables `brookesia_app_settings`, compiled from its pinned source in
`.deps/assembled`. Its package resources are staged into
`brookesia/fs/littlefs/apps`, matching the WASM internal-storage mount used by
SuperOS. Package-app discovery remains disabled.

The executable uses a 256 KiB Emscripten stack because parsing the Settings GUI
documents exceeds Emscripten's 64 KiB default in debug builds.
LVGL's built-in TJPGD decoder and memory-filesystem adapter are enabled for
JPEG resources used by Settings.
The WASM build also enables FreeType so packaged TTF fonts use the same font
metrics as the native build. TAB5's density and font scale come from the same
board-common configuration used by the native firmware.
The SuperOS resource-stage target copies shell assets below the selected board's
build directory, then the executable preloads them as `/brookesia` in the
Emscripten filesystem.
`main.cpp` configures the WASM display, starts and binds the Display service,
enables its backlight, activates the LVGL source role, then defers SuperOS
startup until the browser event loop runs.

The published SuperOS PC route does not add System Core itself, and System Core
does not add Runtime Manager. The example therefore routes both components
explicitly before `brookesia_system_super`.

System Core also requires the Device service during initialization. The executable
links `brookesia::service_device` with `WHOLE_ARCHIVE` so the static service
registration object is retained. Without it, LVGL displays its pale background but
SuperOS stops before mounting the desktop.

Every allowlisted app must also have a pinned entry in `wasm-components.lock.json`.
Configuration fails with a `fetch.ps1` instruction when its assembled source is
missing.

The target configures, compiles, stages its shell resources, and links successfully
with Emscripten.

## WASM resource handling

The forked SuperOS component provides `resource_root_path`, and this example sets
it to `/brookesia`. The forked WASM HAL registers that preloaded directory with
the storage filesystem interface. Together they keep packaged, read-only shell
resources separate from writable emulated storage.
