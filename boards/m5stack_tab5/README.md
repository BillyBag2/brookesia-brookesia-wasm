# M5Stack TAB5 board support

This directory keeps the native TAB5 board description beside the WASM implementation derived from it.

## Layout

- `source/` contains the authoritative TAB5 YAML inputs copied from the M5Stack Boards component.
- `generated/` contains a reference snapshot produced by ESP Board Manager. These files document the native device and peripheral graph; they are not compiled into the WASM target.
- `wasm/` contains project-owned, ESP-IDF-free implementations of the TAB5-facing behaviour used in a browser.

The root `bb-superos` CMake project selects this implementation with
`-DBROOKESIA_BOARD=m5stack_tab5`. The PowerShell wrapper exposes the same setting
as `build-bb-superos.ps1 -Board m5stack_tab5` and uses the isolated build directory
`build/bb-superos/m5stack_tab5`.

## Upstream provenance

The current snapshot came from the `brookesia-brookesia` submodule at commit `364aafb64b580c9d2b1b1b717a4dbb683a42957a`.

- `espressif/m5stack_boards`: version `0.5.4`, upstream commit `75822eaf00d76f1adb4df13f369cec7`
- `espressif/esp_board_manager`: version `0.5.15`, upstream commit `cbee842b9fb94703b5b2ee36aa7bd5a256f16744fcc63eaf3cf995e6fe3e`

The exact component versions must be verified against `brookesia-brookesia/dependencies.lock` whenever this snapshot is updated. The component manifests are in `brookesia-brookesia/managed_components/`.

## Updating the snapshot

The root WASM build does not require ESP-IDF. ESP-IDF is needed only when deliberately refreshing this native reference snapshot.

From PowerShell, initialise the ESP-IDF environment and build the native submodule for the TAB5:

```powershell
cd brookesia-brookesia
& "$env:IDF_PATH\export.ps1"
idf.py set-target esp32p4
idf.py reconfigure
$env:PYTHONUTF8 = "1"
python managed_components/espressif__esp_board_manager/gen_bmgr_config_codes.py -b m5stack_tab5
idf.py build
```

Then update `source/` from:

```text
brookesia-brookesia/managed_components/espressif__m5stack_boards/m5stack_tab5/
```

Copy `board_info.yaml`, `board_devices.yaml`, and `board_peripherals.yaml`. Update `generated/` from:

```text
brookesia-brookesia/components/gen_bmgr_codes/
```

Copy the `gen_board_*.c` files and `gen_board_metadata.yaml`. Do not copy its generated `CMakeLists.txt`: it contains native ESP-IDF integration and may contain an absolute path from the machine that ran the generator.

Finally, update the submodule commit and component versions recorded above, review the diff, and confirm that values represented in `wasm/` still match the refreshed definitions.

## WASM mapping

The physical TAB5 display is 720 pixels wide by 1280 pixels high in its native portrait orientation and uses RGB565. The WASM implementation starts with those properties. Browser canvas output and pointer input will eventually stand in for the native DSI display and I2C touch controller; unsupported physical peripherals should be represented by explicit browser implementations or stubs rather than ESP-IDF drivers.

The board exposes these properties through the stable
`brookesia/board_wasm/config.hpp` include path. The SuperOS launcher uses that
common API, so selecting another board changes the display configuration without
requiring a board-specific include in `main.cpp`. `DisplayWasmDevice` passes the
selected width and height through its panel and touch interfaces; LVGL and the
SuperOS environment then obtain the same dimensions from the active display.

Appearance settings shared with the native firmware come directly from
`brookesia-brookesia/boards/m5stack_tab5/common/include/brookesia/board/config.hpp`.
The native and WASM launchers both consume its density and font scale, while
hardware properties such as resolution and pixel format remain sourced from the
native BSP or the WASM hardware description. ESP-IDF-only preparation is isolated
under the submodule board's `native/` directory.

`wasm/apps.cmake` is the manually maintained allowlist of registered apps known to
work in this board's browser build. App implementations are resolved from the
version-locked `.deps/assembled` tree; selecting an app does not compile the
submodule's generated `managed_components` directory directly.
