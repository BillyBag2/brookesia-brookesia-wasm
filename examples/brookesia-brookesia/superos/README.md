# Brookesia-Brookesia SuperOS WASM experiment

This is the published-component counterpart to `examples/esp-brookesia/superos`.
It sources SuperOS, System Core, GUI, services, and LVGL from `.deps/assembled`.
The fetch lock obtains `brookesia_hal_wasm` and `brookesia_system_super` from a
fixed commit of the `BillyBag2/esp-brookesia` fork. The remaining Brookesia
components come from fixed ESP Component Registry releases.

## Current route

`CMakeLists.txt` adds the selected component projects directly and builds
`bb-superos.html`. It deliberately disables registered and package app discovery;
the acceptance target is the same intentionally empty desktop as `eb-superos`.
The SuperOS resource-stage target copies shell assets to `build/bb-superos/brookesia`,
then the executable preloads them as `/brookesia` in the Emscripten filesystem.
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

The target configures, compiles, stages its shell resources, and links successfully
with Emscripten.

## WASM resource handling

The forked SuperOS component provides `resource_root_path`, and this example sets
it to `/brookesia`. The forked WASM HAL registers that preloaded directory with
the storage filesystem interface. Together they keep packaged, read-only shell
resources separate from writable emulated storage.
