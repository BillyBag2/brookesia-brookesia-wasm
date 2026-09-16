# Brookesia WebAssembly

This project runs Brookesia/LVGL interfaces and a basic Brookesia-Brookesia
SuperOS desktop in a web browser. The browser build uses Emscripten and shares
portable C++ application, appearance, and board configuration with the native
ESP-IDF firmware where practical.

## What works

- The Brookesia layout example builds and runs in the browser with pointer input.
- The upstream-style ESP-Brookesia SuperOS example (`eb-superos`) reaches its
  desktop.
- The Brookesia-Brookesia SuperOS example (`bb-superos`) reaches the TAB5-shaped
  portrait desktop and can launch the Settings app.
- Settings resources, fonts, PNG and JPEG images, and the LittleFS application
  tree are packaged into Emscripten's virtual filesystem.
- The selected board supplies common native/WASM appearance settings and WASM
  display dimensions. `m5stack_tab5` is currently the only board target.
- Browser display, touch/pointer input, storage, and the service interfaces needed
  by the current desktop and Settings flow have WASM implementations or shims.
- Dependency versions and project patches are reproducibly assembled under
  `.deps` from `wasm-components.lock.json`.
- PowerShell scripts fetch dependencies, build each example, and assemble a
  static site in `output`.
- GitHub Actions builds all three examples for pull requests and `main`, uploads
  the site as an artifact, and publishes successful `main` builds to GitHub Pages.

## Current limitations

WebAssembly is a native compilation target for C and C++, but a browser cannot run
an ESP-IDF firmware image or an ESP-specific ELF application unchanged. Code that
uses FreeRTOS, ESP drivers, NVS, networking, audio, or other ESP-IDF facilities
needs a browser implementation, a stub, or a platform-independent interface.

Brookesia's `brookesia_runtime_elf` component is currently ESP-IDF-only, so ESP ELF
applications cannot be reused in the browser. The framework fork does contain
`brookesia_runtime_wasm`, which executes application packages compiled as
WebAssembly. This is distinct from compiling Brookesia itself to WebAssembly with
Emscripten. The runtime backend and its underlying WASM engine have not yet been
verified when nested inside the browser build.

The current browser applications are statically linked into the main Emscripten
module. Loading independently packaged applications through
`brookesia_runtime_wasm` inside the browser build has not been validated.

Audio capture and playback are not implemented for the browser HAL. Native audio
also depends on completing and validating the target's hardware driver path.
Networking, persistent browser storage, keyboard and wheel input, and the full
native application list are not yet supported or fully tested. Browser performance
and rendering can also differ from the ESP32/PPA path used by the native target.

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

The WASM build assembles a correctly shaped source tree without invoking
ESP-IDF or using the firmware project's generated `managed_components` directory.
A committed `wasm-components.lock.json` lists every source, exact version or Git
commit, integrity hash, and destination. The standalone fetch script creates the
tree under `.deps/`; that directory is generated and should not be committed.
The lock file also lists repository-owned patches applied to assembled components.
Patch application is idempotent and fails if a pinned dependency no longer matches,
so compatibility changes remain reproducible instead of becoming untracked edits
inside `.deps`.

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
only by the browser build. This includes `brookesia_runtime_wasm` and its
`wasm-micro-runtime` dependency, although the current statically linked examples do
not use the nested application runtime.

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
to this project. It adds the generated components in dependency order and uses the
upstream top-level CMake file as a
reference. This avoids coupling released components to whatever component versions
happen to be present on the fork's `master` branch.

## SuperOS component builds

There are two SuperOS browser targets. `eb-superos` provides the framework-oriented
desktop reference. `bb-superos` reproduces the board-selected product desktop and
includes its selected registered applications and packaged resources; TAB5
currently includes Settings.

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

## Layout browser example

The `brookesia_layout_test` target uses Brookesia's existing GUI example registry
and runner. The
example menu covers JSON layouts, widgets, styles, events, and pointer input, so it
is a useful focused test of the shared UI code. It keeps Wi-Fi and other product
services out of this smaller build; the complete Settings integration is exercised
by `bb-superos`.

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
`build`, copies them to the ignored `output` directory, creates `output/index.html`
from `cmake/wasm-index.html.in`, and opens that page in the default browser. Edit
that template to change the landing-page presentation or repository links. Matching `.data` archives and optional
`*-host.html` launchers are copied too; the index prefers the cleaner host page.
It serves `output` until you press Ctrl+C. Most Windows terminals make the printed URL
clickable. Use `host.ps1 -Port 8080` to select another port or
`host.ps1 -NoBrowser` to start the server without opening a browser. Do not open
the HTML directly from the filesystem because browsers restrict resources loaded
by WASM.

Use `host.ps1 -PublishOnly` to assemble the same static `output` directory without
starting a server or opening a browser. The `Build and publish WASM examples`
GitHub Actions workflow fetches the locked dependencies, builds `bb-layout`,
`eb-superos`, and the `m5stack_tab5` `bb-superos`, then runs this publishing mode.
Every successful workflow uploads `output` as a downloadable artifact. Successful
`main` builds also deploy that exact directory to the repository's GitHub Pages
site; pull requests build and upload an artifact without deploying it.

The build deliberately consumes only `.deps/assembled`; it does not use ESP-IDF or
compile against the reference submodule. Delete `build/bb-layout` whenever a completely
fresh CMake configuration is required. Use `fetchClean.ps1` only when the fetched
source tree itself must also be recreated.

## Future work

- Complete the browser HAL for audio, networking, persistent storage, and any
  additional device services required by new applications.
- Finish and validate native TAB5 audio drivers, keeping portable application code
  shared with the browser build.
- Add more applications selectively through each board's `apps.cmake` rather than
  assuming every native application is browser-compatible.
- Test `brookesia_runtime_wasm` as a nested runtime and establish a supported
  package format for independently built browser applications.
- Add keyboard, wheel, clipboard, and other browser integrations when an
  application needs them.
- Profile startup, rendering, memory consumption, and download size, then add a
  release-oriented Emscripten configuration.
- Compare native and browser screenshots and interactions at the same board
  resolution, including fonts, touch targets, transitions, and resources.
- Add automated browser smoke tests and retain native firmware builds as the
  hardware parity check.
- Add further boards by supplying common appearance configuration, native board
  integration, a WASM board target, and a separate build directory.

## Repository layout

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

The generated `.deps`, `build`, and `output` directories can be recreated from the
checked-in lock file, build scripts, and source configuration.
