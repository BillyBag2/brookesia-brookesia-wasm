[CmdletBinding()]
param(
    [string]$Preset = "wasm-debug",
    [string]$EmsdkPath = "C:\tools\emsdk",
    [switch]$Fresh
)

$ErrorActionPreference = "Stop"

$emsdkEnvironment = Join-Path $EmsdkPath "emsdk_env.ps1"
if (-not (Test-Path -LiteralPath $emsdkEnvironment -PathType Leaf)) {
    throw "Emscripten environment script not found: $emsdkEnvironment. Pass -EmsdkPath if emsdk is installed elsewhere."
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake was not found on PATH. Install CMake and open a new terminal before building."
}

# Ninja installed by ESP-IDF is often not on PATH outside an ESP-IDF terminal.
if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    $ninjaSearchRoots = @(
        (Join-Path $env:USERPROFILE ".espressif\tools\ninja"),
        "C:\Espressif\tools\ninja"
    )
    $ninja = $ninjaSearchRoots |
        Where-Object { Test-Path -LiteralPath $_ -PathType Container } |
        ForEach-Object { Get-ChildItem -LiteralPath $_ -Filter "ninja.exe" -Recurse -File -ErrorAction SilentlyContinue } |
        Sort-Object FullName -Descending |
        Select-Object -First 1

    if ($null -eq $ninja) {
        throw "Ninja was not found on PATH or in the standard ESP-IDF tool directories."
    }
    $env:PATH = "$($ninja.DirectoryName);$env:PATH"
    Write-Host "Using ESP-IDF Ninja: $($ninja.FullName)"
}

$requiredSources = @(
    (Join-Path $PSScriptRoot ".deps\assembled\esp-brookesia\gui\brookesia_gui_lvgl\CMakeLists.txt"),
    (Join-Path $PSScriptRoot ".deps\assembled\external\lvgl\CMakeLists.txt")
)
foreach ($source in $requiredSources) {
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        throw "Fetched WASM sources are missing. Run .\fetch.ps1 before building."
    }
}

. $emsdkEnvironment

$configureArguments = @("cmake", "--preset", $Preset)
if ($Fresh) {
    $configureArguments += "--fresh"
}

Push-Location $PSScriptRoot
try {
    & emcmake @configureArguments
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed with exit code $LASTEXITCODE."
    }

    & cmake --build --preset $Preset
    if ($LASTEXITCODE -ne 0) {
        throw "CMake build failed with exit code $LASTEXITCODE."
    }
} finally {
    Pop-Location
}

Write-Host "Built build-wasm\brookesia_layout_test.html"
