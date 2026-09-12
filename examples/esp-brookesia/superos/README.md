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

- The executable and its 2.1 MB resource archive build successfully. Browser
  startup and rendering have not yet been verified.
- The upstream SuperOS lifecycle uses asynchronous services and browser main
  thread execution. If startup blocks, locate the blocking call before adding
  another Emscripten loop; `DisplaySource` already owns one.
- App discovery is intentionally disabled. Enable one app only after the base
  shell is visible, then add its dependent services and staged resources.
- The upstream JavaScript runtime is not enabled in this minimal target, so
  JavaScript package apps are out of scope for this first executable.
- `thirdparty/esp-brookesia` still uses deprecated `FetchContent_Populate` when
  the JavaScript runtime is enabled. That is upstream work and is not changed
  here.
