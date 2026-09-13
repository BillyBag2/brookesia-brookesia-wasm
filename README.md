# Brookesia WebAssembly

This project aims to run brookesia-brookesia in a web browser. The
first target is a fast layout and interaction preview for Brookesia/LVGL user
interfaces. Application logic written in portable C++ can run in the browser when
it is compiled to WebAssembly with Emscripten.

## Important constraint

WebAssembly is a native compilation target for C and C++, but a browser cannot run
an ESP-IDF firmware image or an ESP-specific ELF application unchanged. Code that
uses FreeRTOS, ESP drivers, NVS, networking, audio, or other ESP-IDF facilities
needs a browser implementation, a stub, or a platform-independent interface.

Brookesia's `brookesia_runtime_elf` component is currently ESP-IDF-only, so ESP ELF
applications cannot be reused in the browser. The framework fork does contain
`brookesia_runtime_wasm`, which executes application packages compiled as
WebAssembly. This is distinct from compiling Brookesia itself to WebAssembly with
Emscripten. The port needs to verify that the runtime backend and its underlying
WASM engine work when nested inside the browser build.

For the first visual milestone, statically linking one C++ demo app remains the
simplest route. The next milestone should compile a portable C++ app against the
Brookesia WASM application ABI and load it through `brookesia_runtime_wasm`. Avoid
inventing an Emscripten dynamic-linking ABI unless the existing application runtime
cannot support the browser target.

## Repository dependencies

The existing `brookesia-brookesia` submodule is the ESP-IDF firmware project and
should remain here as the device implementation and parity reference. It currently
uses the Brookesia 0.8 components from the ESP Component Registry; it is not the
source repository for the Brookesia framework itself.

After cloning this project, initialise the firmware submodule with:

```sh
git submodule update --init --recursive
```

or on Windows PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\fetchSubmodules.ps1
```

The browser build also needs the Brookesia framework source. Your fork of
[`espressif/esp-brookesia`](https://github.com/espressif/esp-brookesia) is now
present at `thirdparty/esp-brookesia`. It is the reference for the upstream source
layout and host CMake build. Normal WASM builds use fixed Registry releases where
possible. Two components are intentionally fetched directly from the fork at a
fixed commit: `brookesia_hal_wasm` and `brookesia_system_super`. They contain the
matching WASM resource-storage changes not yet available together as Registry
releases. The build does not read these files from the local `thirdparty` checkout.

Do not replace the existing `brookesia-brookesia` submodule with that framework
fork: the two repositories have different roles.

The checked-in [`boards/m5stack_tab5`](boards/m5stack_tab5/README.md) directory
records the native TAB5 board definition and generated Board Manager output used
as the parity reference for browser support. Its `wasm/` directory contains the
ESP-IDF-free browser implementation. The common
`brookesia/board_wasm/config.hpp` API propagates the selected board's display
configuration to the SuperOS launcher; TAB5 currently supplies its native
720 x 1280 RGB565 display. See the board README for component provenance and
snapshot update instructions. Refreshing that reference requires ESP-IDF, but
normal WASM fetching and builds do not.

Portable product appearance settings are shared directly from the firmware
submodule at `brookesia-brookesia/boards/<board>/common`. Both native and WASM
launchers consume that board profile, while physical hardware properties continue
to come from the native BSP and generated Board Manager data.

Each board may select a tested subset of registered browser apps in
`boards/<board>/wasm/apps.cmake`. Those names resolve only through the pinned
`.deps/assembled` tree. Add a matching entry to `wasm-components.lock.json` before
selecting a new app; configuration reports missing fetched sources and directs you
to run `fetch.ps1`.

## WASM source tree and fetching

The WASM build should assemble a correctly shaped source tree without invoking
ESP-IDF or using the firmware project's generated `managed_components` directory.
A committed `wasm-components.lock.json` lists every source, exact version or Git
commit, integrity hash, and destination. A standalone fetch script will create the
tree under `.deps/`; that directory is generated and should not be committed.

Run the fetcher from the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\fetch.ps1
```

The process-scoped execution-policy option is useful on Windows hosts that block
local scripts; it does not change the machine or user policy. If local scripts are
already permitted, `.\fetch.ps1` is equivalent. The wrapper invokes
`scripts/fetch-wasm-components.ps1`; its optional `-LockFile` parameter selects a
different lock file.

An existing component directory is treated as complete and up to date. This keeps
normal runs fast and preserves any local investigation under `.deps`. To discard
the entire generated tree, cached downloads, and any local changes, then fetch and
verify everything again, run:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\fetchClean.ps1
```

`-Clean` is deliberately limited to this repository's `.deps` directory. Source
submodules and project files are never removed by the script. If local scripts are
already permitted, the short command is `.\fetchClean.ps1`.

There are four kinds of generated directory. Keeping them separate makes the
origin of every file visible and lets the assembled tree be deleted and rebuilt at
any time.

```text
.deps/
  cache/
    registry/                           # downloaded Registry archives
    git/                                # temporary Git object cache

  sources/
    registry/
      managed_components/               # mirrors the firmware's resolved set
        espressif__brookesia_app_settings/
        espressif__brookesia_gui_interface/
        espressif__brookesia_gui_lvgl/
        espressif__brookesia_hal_interface/
        espressif__brookesia_lib_utils/
        espressif__brookesia_runtime_manager/
        espressif__brookesia_service_device/
        espressif__brookesia_service_display/
        espressif__brookesia_service_helper/
        espressif__brookesia_service_manager/
        espressif__brookesia_service_sntp/
        espressif__brookesia_service_storage/
        espressif__brookesia_service_wifi/
        espressif__brookesia_system_core/
        espressif__brookesia_system_super/
        lvgl__lvgl/

      additional_components/            # published components needed only by WASM
        espressif__brookesia_runtime_wasm/
        espressif__wasm-micro-runtime/

    git/
      esp-brookesia-<commit>/            # sparse checkout of unpublished sources
        hal/
          brookesia_hal_wasm/

  assembled/
    esp-brookesia/                       # source tree consumed by WASM CMake
      app/
        brookesia_app_settings/          # from Registry managed_components
      gui/
        brookesia_gui_interface/         # from Registry; includes wasm_shims
        brookesia_gui_lvgl/              # from Registry; includes src/port/wasm
      hal/
        brookesia_hal_interface/         # from Registry managed_components
        brookesia_hal_wasm/              # copied from pinned Git source
      runtime/
        brookesia_runtime_manager/       # from Registry managed_components
        brookesia_runtime_wasm/          # from additional Registry components
      service/
        framework/
          brookesia_service_manager/     # from Registry managed_components
          brookesia_service_helper/      # from Registry managed_components
        media/
          brookesia_service_display/     # from Registry managed_components
        system/
          brookesia_service_storage/     # from Registry managed_components
      system/
        brookesia_system_core/           # from Registry managed_components
        brookesia_system_super/          # from Registry managed_components
      utils/
        brookesia_lib_utils/             # from Registry; includes wasm_shims
    external/
      lvgl/                              # from Registry lvgl__lvgl
      wasm-micro-runtime/                # only when runtime_wasm is enabled
```

`sources/registry/managed_components` reproduces the relevant part of the ESP-IDF
Component Manager's flat naming scheme, using the versions resolved for the
firmware. It is produced directly from Registry archives; it is not copied from the
firmware checkout. Hardware-only entries from the firmware's complete dependency
set do not need to be downloaded.

`sources/registry/additional_components` contains published dependencies required
only by the browser build. Initially this means `brookesia_runtime_wasm` and its
`wasm-micro-runtime` dependency, and neither is required for the first statically
linked layout milestone.

`sources/git` contains components fetched from pinned Git commits. The WASM HAL and
SuperOS component are obtained from the `BillyBag2/esp-brookesia` fork at the same
fixed commit so their resource-storage APIs stay in sync.

`assembled` reconstructs the paths used by the ESP-Brookesia monorepo. The fetch
script copies verified sources into it; CMake never needs to know whether a
component originated in the Registry or Git. Nothing under `.deps` should be
edited by hand.

The initial Registry versions should match
`brookesia-brookesia/dependencies.lock`. The WASM lock may add published components
that the firmware does not use, such as `brookesia_runtime_wasm`, and should omit
ESP-only board, LCD, camera, Wi-Fi coprocessor, and filesystem components. Each
Registry archive contains its own `CHECKSUMS.json`; the fetcher must validate that
and the component hash before installing the source.

`brookesia_hal_wasm` and `brookesia_system_super` are currently the exceptions.
The HAL does not have a published Registry component, and the forked SuperOS
component contains the matching resource-root change. Their lock entries name the
fork, the same full commit SHA, their source paths, and their generated destinations:

```yaml
- name: brookesia_hal_wasm
  source: git
  repository: https://github.com/BillyBag2/esp-brookesia.git
  commit: 3312eee45c6bab78ae1f4dd5942afda9cb06d989
  path: hal/brookesia_hal_wasm
  destination: esp-brookesia/hal/brookesia_hal_wasm
- name: brookesia_system_super
  source: git
  repository: https://github.com/BillyBag2/esp-brookesia.git
  commit: 3312eee45c6bab78ae1f4dd5942afda9cb06d989
  path: system/brookesia_system_super
  destination: esp-brookesia/system/brookesia_system_super
```

Pinning the commit makes normal fetches reproducible and prevents later fork changes
from silently entering the build. The remaining Brookesia components continue to
come from the versions and hashes pinned in the Registry section of the lock file.

Git cannot clone only a repository subdirectory. A sparse checkout still has one
Git repository, but materialises only selected paths in its working tree. The fetch
script can use a temporary sparse checkout and copy the required directory into the
generated tree:

```sh
git init .cache/esp-brookesia
git -C .cache/esp-brookesia remote add origin https://github.com/BillyBag2/esp-brookesia.git
git -C .cache/esp-brookesia sparse-checkout init --cone
git -C .cache/esp-brookesia sparse-checkout set <component-path>
git -C .cache/esp-brookesia fetch --depth 1 origin <commit-sha>
git -C .cache/esp-brookesia checkout --detach FETCH_HEAD
```

A submodule cannot point directly at `hal/brookesia_hal_wasm`; a submodule records
an entire repository and commit, with the repository root mounted at its configured
path. The existing full `thirdparty/esp-brookesia` submodule is therefore suitable
as a reference and development checkout. The generated sparse copy is better for
assembling the locked WASM dependency tree.

The layout example's `examples/brookesia-brookesia/layout/CMakeLists.txt` belongs
to this project. It should add the generated
components in dependency order and use the upstream top-level CMake file as a
reference. This avoids coupling released components to whatever component versions
happen to be present on the fork's `master` branch.

## SuperOS component builds

There are two minimal SuperOS targets. Both deliberately disable registered and
package app discovery; their acceptance target is an empty but fully initialized
SuperOS desktop.

- `build-eb-superos.ps1` builds `examples/esp-brookesia/superos` from the complete
  `thirdparty/esp-brookesia` checkout.
- `build-bb-superos.ps1` builds `examples/brookesia-brookesia/superos` from the
  locked `.deps/assembled` tree. Most components are Registry releases, while its
  WASM HAL and SuperOS sources come from the pinned fork commit.

Initialize the submodules and run `fetch.ps1` before building. Use `-Fresh` to
reset CMake configuration or `-EmsdkPath` to select another SDK. `bb-superos`
defaults to the `m5stack_tab5` board and places each board configuration in its
own directory, such as `build/bb-superos/m5stack_tab5`. Select a board with:

```powershell
.\build-bb-superos.ps1 -Board m5stack_tab5
```

The equivalent CMake cache setting is `-DBROOKESIA_BOARD=m5stack_tab5`; use a
matching board-specific build directory when configuring manually. A selectable
board must provide `boards/<board>/wasm/CMakeLists.txt` and the common
`brookesia::board_wasm` CMake target. Its public include directory must provide
`brookesia/board_wasm/config.hpp`, which is the stable API consumed by the
launcher. For example:

```powershell
emcmake cmake -S examples/brookesia-brookesia/superos `
    -B build/bb-superos/m5stack_tab5 -G Ninja `
    -DBROOKESIA_BOARD=m5stack_tab5 -DCMAKE_BUILD_TYPE=Debug
cmake --build build/bb-superos/m5stack_tab5
```

Both builds stage the SuperOS resources and preload them at `/brookesia`;
writable internal storage
uses `/brookesia/fs/littlefs`. The `bb-superos` target explicitly retains the
Device service registration required by System Core.

`host.ps1` discovers and serves all complete browser builds under `build`, including
both SuperOS targets and the layout example. For SuperOS it links to the clean
`*-host.html` launcher while preserving Emscripten's generated `.html` page beside
it for diagnostics.

## Build tools

Install CMake using the Windows x64 installer from the official
[CMake download page](https://cmake.org/download/). During installation, select
the option to add CMake to `PATH`, then open a new terminal and check the
installation:

```powershell
cmake --version
```

Install Ninja with Windows Package Manager, then open a new terminal and verify
that it is available on `PATH`:

```powershell
winget install --exact --id Ninja-build.Ninja
ninja --version
```

If `winget` is unavailable, download `ninja-win.zip` from the official
[Ninja releases page](https://github.com/ninja-build/ninja/releases), extract
`ninja.exe`, and add its directory to `PATH`.

Install the Emscripten SDK at the location used by the build scripts and activate
the project's tested version:

```powershell
git clone https://github.com/emscripten-core/emsdk.git C:\tools\emsdk
Set-Location C:\tools\emsdk
.\emsdk install 6.0.9
.\emsdk activate 6.0.9
. .\emsdk_env.ps1
emcc --version
```

The environment setup applies to the current PowerShell session. The project build
scripts run `emsdk_env.ps1` themselves, so permanent environment changes are not
required. If you install the SDK somewhere else, pass that directory with
`-EmsdkPath`, for example `build-bb-layout.ps1 -EmsdkPath C:\path\to\emsdk`.

Git is required by the source fetcher, PowerShell runs the project scripts, and
Python can serve the generated browser files.

This project has been built on Windows with:

| Tool | Tested version |
| --- | --- |
| CMake | 4.4.3 |
| Emscripten (`emcc`) | 6.0.9 |
| Git for Windows | 2.51.0.windows.2 |
| Windows PowerShell | 5.1.26100.9278 |
| Python | 3.13.11 |

## First browser test

The first target is `brookesia_layout_test`. It uses Brookesia's existing GUI
example registry and runner rather than choosing a full product app yet. The
example menu covers JSON layouts, widgets, styles, events, and pointer input, so it
is a useful test of the shared UI code. It also keeps Wi-Fi and other application
services out of the first build. Once this works, `brookesia_app_settings` is a
good first complete app because it is already used by the firmware project.

Fetch the locked sources and build:

```powershell
.\fetch.ps1
.\build-bb-layout.ps1
```

If Windows PowerShell reports that running scripts is disabled, use a
process-scoped execution-policy bypass:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\fetch.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\build-bb-layout.ps1
```

This applies only to the new PowerShell process and does not change the machine
or user execution policy. Do not add a trailing `\` after the script filename.

`build-bb-layout.ps1` activates Emscripten, configures the `wasm-debug` preset in
`examples/brookesia-brookesia/layout`, and builds the layout example. Its
`CMakeLists.txt` and `CMakePresets.json` live alongside `main.cpp`; output remains
in the repository's `build/bb-layout` directory.
Use `build-bb-layout.ps1 -Fresh` to discard CMake's cached configuration. Run this once
after moving from the former root CMake project to clear its old source path.
If emsdk is installed somewhere else, use
`build-bb-layout.ps1 -EmsdkPath C:\path\to\emsdk`.

`emcmake` selects Emscripten's C and C++ compilers. CMake is a separate host tool
and must also be available on `PATH`. The VS Code CMake Tools extension provides
editor integration but does not install CMake itself. The output is
`build/bb-layout/bb-layout.html` with its JavaScript and WebAssembly files.
Launch the browser test and its local server with:

```powershell
.\host.ps1
```

If local scripts are blocked, use:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\host.ps1
```

The script finds every complete HTML, JavaScript, and WebAssembly triplet under
`build`, copies them to the ignored `output` directory, creates `output/index.html`,
and opens that page in the default browser. Matching `.data` archives and optional
`*-host.html` launchers are copied too; the index prefers the cleaner host page.
It serves `output` until you press Ctrl+C. Most Windows terminals make the printed URL
clickable. Use `host.ps1 -Port 8080` to select another port or
`host.ps1 -NoBrowser` to start the server without opening a browser. Do not open
the HTML directly from the filesystem because browsers restrict resources loaded
by WASM.

The build deliberately consumes only `.deps/assembled`; it does not use ESP-IDF or
compile against the reference submodule. Delete `build/bb-layout` whenever a completely
fresh CMake configuration is required. Use `fetchClean.ps1` only when the fetched
source tree itself must also be recreated.

## Roadmap

### 1. Define the first runnable target

- Build the Brookesia GUI example menu as the first acceptance test.
- Fix the logical display size, colour depth, input methods, and required assets.
- List which parts of the screen use portable Brookesia/LVGL APIs and which call
  ESP-IDF directly.

### 2. Establish the Emscripten build

- Install and pin Emscripten (`emsdk`) and use CMake for the browser build.
- Reproduce the dependency ordering and `BROOKESIA_BUILD_*` options from the
  framework's host CMake build against the generated, version-locked tree.
- First compile its default host components with Emscripten, then a minimal program
  linked to `brookesia::all` into `.wasm`, JavaScript, and HTML.
- Add development and release presets; enable useful assertions and source maps in
  development builds.
- Serve the output through a local HTTP server rather than opening the HTML file
  directly.

### 3. Bring up the existing WASM HAL and LVGL

- Enable `BROOKESIA_BUILD_HAL_WASM` and validate the existing SDL2 display and input
  implementation. It already selects Emscripten SDL2, Fetch, Asyncify, and memory
  growth options.
- Enable `BROOKESIA_BUILD_GUI_LVGL`, provide its required `lvgl` CMake target, and
  render to the browser canvas.
- Verify that pointer/touch events from `DisplayWasmDevice` reach LVGL; add keyboard
  and wheel mappings only if the chosen UI needs them.
- Drive `lv_timer_handler()` from an Emscripten animation-frame callback so the
  browser event loop is never blocked.
- Verify resizing, display scaling, frame timing, and basic input before adding
  Brookesia.

### 4. Compile the required Brookesia components

- Start with the existing host defaults, then enable `brookesia_hal_wasm`,
  `brookesia_gui_lvgl`, `brookesia_system_core`, and `brookesia_system_super` in
  dependency order.
- Use the existing `BROOKESIA_BUILD_*` CMake options and exported
  `brookesia::<component>` targets.
- Isolate ESP-IDF headers and APIs behind feature checks and platform adapters.
- Prefer small portability changes in the Brookesia fork that can later be offered
  upstream.

### 5. Complete the platform abstraction

- Audit `brookesia_hal_wasm` coverage for time, tasks, locks, logging, files,
  persistent settings, networking, and every device service used by the selected
  UI application.
- Extend its browser implementations or deterministic simulators where coverage is
  missing. Use the main browser loop for the first milestone; add WebAssembly
  threads only if profiling proves they are needed.
- Keep ESP-IDF implementations behind the same interfaces so application and UI
  code can build for both targets.

### 6. Package resources

- Reuse Brookesia's GUI documents and LVGL asset conversion where it is portable.
- Package fonts, images, JSON, and application resources into Emscripten's virtual
  filesystem.
- Use browser persistence such as IDBFS only for data that must survive reloads.
- Check asset paths, colour formats, memory use, and initial download size.

### 7. Port C++ applications

- Define a small lifecycle such as `create`, `start`, `update`, `suspend`, and
  `destroy` that does not expose ESP-IDF types.
- Compile the first applications into the main WebAssembly module and register
  them with a browser-compatible launcher.
- Replace direct hardware access with injected services or desktop/browser mocks.
- Enable `brookesia_runtime_manager` and `brookesia_runtime_wasm`, then verify the
  existing WASM application package lifecycle inside the Emscripten build.
- Build a second portable C++ app as a guest WASM application using the runtime's
  supported ABI and packaging tools. Test loading, events, cleanup, errors, and
  repeated launches.

### 8. Add browser integration

- Provide a small HTML/JavaScript shell containing the canvas, loading state,
  console output, and optional device controls.
- Add JavaScript bindings only at the platform boundary; keep layout and
  application behaviour in shared C++.
- Handle focus, clipboard, browser storage, URL parameters, and error reporting as
  needed by the chosen demo.

### 9. Verify parity

- Run the same UI scenario on ESP hardware and in the browser.
- Compare screenshots at the target resolution and test navigation, touch targets,
  animations, fonts, and resource loading.
- Add automated browser smoke tests and a build check for both Emscripten and the
  existing ESP-IDF project.

### 10. Publish a repeatable preview

- Pin Emscripten, LVGL, and Brookesia revisions.
- Document one-command configure, build, and serve workflows.
- Add CI that builds the WebAssembly bundle and publishes a static preview, for
  example with GitHub Pages.

## Suggested project shape

```text
brookesia-brookesia/   # existing ESP-IDF firmware project (submodule)
thirdparty/
  esp-brookesia/       # upstream reference and development fork (submodule)
.deps/                 # generated, version-locked WASM source tree
build/                 # generated outputs (ignored by Git)
  bb-layout/
  eb-superos/
cmake/                 # host and Emscripten build helpers
scripts/
  fetch-wasm-components.ps1
fetch.ps1
fetchClean.ps1
build-bb-layout.ps1
build-eb-superos.ps1
host.ps1
wasm-components.lock.json
platform/
  esp_idf/             # existing device adapters
  web/                 # browser adapters and JS bindings
apps/                  # portable C++ demo applications
web/                   # HTML/CSS/JavaScript shell
examples/
  brookesia-brookesia/
    layout/            # main.cpp, CMakeLists.txt, and CMakePresets.json
  esp-brookesia/       # upstream examples
```

The first useful milestone is deliberately narrow: one statically linked C++ app,
one Brookesia screen, mouse/touch input, and its real assets rendered in a browser.
That milestone will expose the actual ESP-IDF dependencies before enabling and
testing the existing WASM application runtime.
