[CmdletBinding()]
param(
    [string]$EmsdkPath = "C:\tools\emsdk",
    [switch]$Fresh
)
$ErrorActionPreference = "Stop"
$sourceDirectory = Join-Path $PSScriptRoot "examples\esp-brookesia\superos"
$buildDirectory = Join-Path $PSScriptRoot "build/eb-superos"
foreach ($tool in @("cmake", "ninja")) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool was not found on PATH. Install it before building."
    }
}
if (-not (Get-Command emcmake -ErrorAction SilentlyContinue)) {
    $emsdkEnvironment = Join-Path $EmsdkPath "emsdk_env.ps1"
    if (-not (Test-Path -LiteralPath $emsdkEnvironment -PathType Leaf)) {
        throw "Emscripten is not active and its environment script was not found: $emsdkEnvironment. Pass -EmsdkPath to select your SDK."
    }
    . $emsdkEnvironment
}
$configureArguments = @("cmake", "-S", $sourceDirectory, "-B", $buildDirectory,
    "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Debug")
if ($Fresh) {
    $configureArguments += "--fresh"
}
& emcmake @configureArguments
if ($LASTEXITCODE -ne 0) {
    throw "SuperOS configuration failed with exit code $LASTEXITCODE."
}
& cmake --build $buildDirectory
if ($LASTEXITCODE -ne 0) {
    throw "SuperOS build failed with exit code $LASTEXITCODE."
}
Write-Host "Built $buildDirectory\eb-superos-host.html (generated diagnostic page: eb-superos.html)"
