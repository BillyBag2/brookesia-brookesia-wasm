[CmdletBinding()]
param(
    [string]$LockFile = "wasm-components.lock.json"
)

$ErrorActionPreference = "Stop"
$fetcher = Join-Path $PSScriptRoot "scripts/fetch-wasm-components.ps1"
& $fetcher -Clean -LockFile $LockFile
