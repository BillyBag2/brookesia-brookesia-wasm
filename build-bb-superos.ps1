[CmdletBinding()]
param(
    [string]$EmsdkPath = "C:\tools\emsdk",
    [ValidatePattern("^[A-Za-z0-9][A-Za-z0-9_-]*$")]
    [string]$Board = "m5stack_tab5",
    [switch]$Fresh
)

$ErrorActionPreference = "Stop"
$sourceDirectory = Join-Path $PSScriptRoot "examples\brookesia-brookesia\superos"
$boardDirectory = Join-Path $PSScriptRoot "boards\$Board\wasm"
$buildDirectory = Join-Path $PSScriptRoot "build\bb-superos\$Board"
$emsdkEnvironment = Join-Path $EmsdkPath "emsdk_env.ps1"

if (-not (Test-Path -LiteralPath (Join-Path $boardDirectory "CMakeLists.txt") -PathType Leaf)) {
    throw "WASM board '$Board' was not found at $boardDirectory."
}
if (-not (Test-Path -LiteralPath $emsdkEnvironment -PathType Leaf)) {
    throw "Emscripten environment script not found: $emsdkEnvironment. Pass -EmsdkPath to select your SDK."
}
foreach ($tool in @("cmake", "ninja")) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool was not found on PATH. Install it before building."
    }
}

$requiredSources = @(
    (Join-Path $PSScriptRoot ".deps\assembled\esp-brookesia\system\brookesia_system_super\CMakeLists.txt"),
    (Join-Path $PSScriptRoot ".deps\assembled\esp-brookesia\hal\brookesia_hal_wasm\CMakeLists.txt"),
    (Join-Path $PSScriptRoot ".deps\assembled\external\lvgl\CMakeLists.txt")
)
foreach ($source in $requiredSources) {
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        throw "Fetched WASM sources are missing. Run .\fetch.ps1 before building."
    }
}

. $emsdkEnvironment

$configureArguments = @("cmake", "-S", $sourceDirectory, "-B", $buildDirectory,
    "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Debug", "-DBROOKESIA_BOARD=$Board")
if ($Fresh) {
    $configureArguments += "--fresh"
}

& emcmake @configureArguments
if ($LASTEXITCODE -ne 0) {
    throw "Brookesia-Brookesia SuperOS configuration failed with exit code $LASTEXITCODE."
}

& cmake --build $buildDirectory
if ($LASTEXITCODE -ne 0) {
    throw "Brookesia-Brookesia SuperOS build failed with exit code $LASTEXITCODE."
}

Write-Host "Built $buildDirectory\bb-superos-host.html for board $Board (generated diagnostic page: bb-superos.html)"
