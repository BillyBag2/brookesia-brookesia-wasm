# SuperOS WASM experiment

This example builds `eb-superos.html`, JavaScript, WebAssembly, and a preloaded
`/brookesia` resource tree from the `thirdparty/esp-brookesia` checkout.

The build output is:

```text
build/eb-superos/eb-superos.html
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

- The executable and its 2.1 MB resource archive build successfully. The
  browser now reaches the WASM display and shows its light-grey LVGL
  background. The SuperOS shell has not yet drawn visible content.
- System Core requires the Device service even when no applications are enabled.
  `BROOKESIA_BUILD_SERVICE_DEVICE` is therefore part of the minimal component set,
  and its static registration library is linked with `WHOLE_ARCHIVE`.
- Startup is deferred with `emscripten_async_call` until after `main()` returns,
  because SystemCore uses the browser-backed single-thread task scheduler.
- Storage cannot be omitted as a workaround: Wi-Fi requires it and Display also
  links it as a transitive dependency. During `System::init()`, System Core
  reads storage preferences and enumerates filesystems with synchronous service
  calls. The WASM task scheduler completes service work through a browser
  callback, which blocks the single browser thread.
- The upstream SuperOS lifecycle uses asynchronous services and browser main
  thread execution. If startup blocks, locate the blocking call before adding
  another Emscripten loop; `DisplaySource` already owns one.
- App discovery is intentionally disabled. Enable one app only after the base
  shell is visible, then add its dependent services and staged resources.
- The upstream JavaScript runtime is not enabled in this minimal target, so
  JavaScript package apps are out of scope for this first executable.
- Upstream work: service `call_function_sync` needs to execute inline when its
  target is already the WASM browser thread, or System Core needs asynchronous
  storage initialization. Either change would allow the Storage service back
  into this build.
- `thirdparty/esp-brookesia` still uses deprecated `FetchContent_Populate` when
  the JavaScript runtime is enabled. That is upstream work and is not changed
  here.
