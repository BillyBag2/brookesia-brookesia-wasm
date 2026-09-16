# SuperOS WASM experiment

This example builds `eb-superos.html`, JavaScript, WebAssembly, and a preloaded
`/brookesia` resource tree from the `thirdparty/esp-brookesia` checkout.
It also configures `eb-superos-host.html`, a clean launcher without Emscripten's
logo, controls, or visible console. The generated `eb-superos.html` remains
available as a diagnostic page.

The build output is:

```text
build/eb-superos/eb-superos.html
build/eb-superos/eb-superos-host.html
build/eb-superos/eb-superos.js
build/eb-superos/eb-superos.wasm
build/eb-superos/eb-superos.data
```

Run from the repository root:

```powershell
.\build-eb-superos.ps1 -Fresh
```

The target deliberately enables no built-in apps. `main.cpp` starts the WASM
display, activates the LVGL source, enables the simulated backlight, and starts
`system::super::System` with package and registered-app discovery disabled.

## Current issues and next steps

- The executable and its resource archive build successfully and reaches the
  intentionally empty SuperOS desktop.
- The example maps writable internal storage to StorageWasmDevice's LittleFS
  path, `/brookesia/fs/littlefs`. The preloaded `/brookesia` archive is reserved
  for shell resources. SuperOS's `resource_root_path` override directs packaged
  shell assets to that archive while preserving the normal ESP default, where
  packaged resources live beneath internal storage.
- System Core requires the Device service even when no applications are enabled.
  `BROOKESIA_BUILD_SERVICE_DEVICE` is therefore part of the minimal component set,
  and its static registration library is linked with `WHOLE_ARCHIVE`.
- Startup is deferred with `emscripten_async_call` until after `main()` returns,
  because SystemCore uses the browser-backed single-thread task scheduler.
- Storage cannot be omitted as a workaround: Wi-Fi requires it and Display also
  links it as a transitive dependency. The WASM scheduler already executes
  immediate service calls inline, so no service-scheduler change was needed for
  this startup path.
- SuperOS previously derived both packaged-resource paths and writable app-data
  paths from internal storage. `resource_root_path` separates those concerns
  without changing the ESP default. This API is intended for upstream review.
- The upstream SuperOS lifecycle uses asynchronous services and browser main
  thread execution. If startup blocks, locate the blocking call before adding
  another Emscripten loop; `DisplaySource` already owns one.
- App discovery is intentionally disabled. Enable one app only after the base
  shell is visible, then add its dependent services and staged resources.
- The upstream JavaScript runtime is not enabled in this minimal target, so
  JavaScript package apps are out of scope for this first executable.
- Potential upstream work: if the probes locate a synchronous wait that cannot
  complete on the browser thread, retain ESP behavior and add a WASM-specific
  inline or asynchronous path at that exact boundary. Do not alter generic
  service scheduling until the blocking call is confirmed.
- `thirdparty/esp-brookesia` still uses deprecated `FetchContent_Populate` when
  the JavaScript runtime is enabled. That is upstream work and is not changed
  here.
