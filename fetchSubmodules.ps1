[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"

git -C $PSScriptRoot submodule update --init --recursive
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
