# Brookesia-Brookesia SuperOS WASM experiment

This is the published-component counterpart to `examples/esp-brookesia/superos`.
It sources SuperOS, System Core, GUI, services, and LVGL from `.deps/assembled`.
The WASM HAL remains source-based because it is not currently consumable as a
separately published component.

## Current route

`CMakeLists.txt` adds the published component CMake projects directly and builds
`bb-superos.html`. It deliberately disables registered and package app discovery.
The SuperOS resource-stage target copies shell assets to `build/bb-superos/brookesia`,
then the executable preloads them as `/brookesia` in the Emscripten filesystem.
`main.cpp` configures the WASM display, starts and binds the Display service,
enables its backlight, activates the LVGL source role, then defers SuperOS
startup until the browser event loop runs.

The published SuperOS PC route does not add System Core itself, and System Core
does not add Runtime Manager. The example therefore routes both components
explicitly before `brookesia_system_super`.

The route configures successfully and the published component sources compile
through the SuperOS application object. A local final-link attempt was blocked
from replacing an existing generated `build/bb-superos/bb-superos.wasm` by a
Windows file lock. Release the process holding that generated file before
rebuilding in place. A clean final-link verification remains to be done outside
the interrupted build-runner session used for this experiment.

## Missing pieces

- **Separate resource and writable roots:** published SuperOS derives shell
  resource paths from internal storage. WASM needs packaged assets at
  `/brookesia` and writable data under `/brookesia/fs/littlefs`. This requires
  the `resource_root_path` API added on the `thirdparty/esp-brookesia` WASM
  branch to be released in the published `brookesia_system_super` component.
- **Packaged-resource filesystem:** published `brookesia_hal_wasm` does not
  expose `/brookesia` to the Storage service. The WASM branch adds that mapping;
  it must be published or copied into a local HAL overlay for SuperOS assets to
  be read through the GUI storage API.

## Completion path

Publish a WASM HAL component that includes the packaged-resource mapping, then
publish the SuperOS resource-root API. With those in place, this example can
use `/brookesia` for packaged resources and a separate LittleFS location for
writable system data, as the Espressif-source example already does.
