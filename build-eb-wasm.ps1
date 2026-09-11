cd .\thirdparty\esp-brookesia

. C:\tools\emsdk\emsdk_env.ps1

emcmake cmake `
  -S . `
  -B build-wasm `
  -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DBROOKESIA_BUILD_HAL_WASM=ON `
  -DBROOKESIA_BUILD_SERVICE_DISPLAY=ON

cmake --build build-wasm

cmake --build build-wasm --target help
