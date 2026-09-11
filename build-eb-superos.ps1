[CmdletBinding()]
param(
    [string]$EmsdkPath = "C:\tools\emsdk",
    [switch]$Fresh
)
$ErrorActionPreference = "Stop"
$sourceDirectory = Join-Path $PSScriptRoot "examples\esp-brookesia\superos"
$buildDirectory = Join-Path $PSScriptRoot "build/eb-superos"
$emsdkEnvironment = Join-Path $EmsdkPath "emsdk_env.ps1"
if (-not (Test-Path -LiteralPath $emsdkEnvironment -PathType Leaf)) {
    throw "Emscripten environment script not found: $emsdkEnvironment. Pass -EmsdkPath to select your SDK."
}
foreach ($tool in @("cmake", "ninja")) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool was not found on PATH. Install it before building."
    }
}
. $emsdkEnvironment
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
Write-Host "Built SuperOS components in $buildDirectory"
