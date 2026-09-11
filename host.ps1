[CmdletBinding()]
param(
    [ValidateRange(1, 65535)]
    [int]$Port = 8000,
    [switch]$NoBrowser
)

$ErrorActionPreference = "Stop"
$buildDirectory = Join-Path $PSScriptRoot "build/bb-layout"
$entryPage = Join-Path $buildDirectory "brookesia_layout_test.html"

if (-not (Test-Path -LiteralPath $entryPage -PathType Leaf)) {
    throw "The browser build was not found. Run .\build-bb-layout.ps1 first."
}

$python = Get-Command python -ErrorAction SilentlyContinue
$pythonArguments = @("-m", "http.server", $Port, "--bind", "127.0.0.1")
if ($null -eq $python) {
    $python = Get-Command py -ErrorAction SilentlyContinue
    $pythonArguments = @("-3") + $pythonArguments
}
if ($null -eq $python) {
    throw "Python was not found on PATH. Install Python or add it to PATH."
}

$url = "http://localhost:$Port/brookesia_layout_test.html"
$server = Start-Process `
    -FilePath $python.Source `
    -ArgumentList $pythonArguments `
    -WorkingDirectory $buildDirectory `
    -NoNewWindow `
    -PassThru

try {
    $ready = $false
    for ($attempt = 0; $attempt -lt 50; $attempt++) {
        if ($server.HasExited) {
            throw "The HTTP server exited before it was ready. Port $Port may already be in use."
        }

        $client = [System.Net.Sockets.TcpClient]::new()
        try {
            $connect = $client.BeginConnect("127.0.0.1", $Port, $null, $null)
            if ($connect.AsyncWaitHandle.WaitOne(100) -and $client.Connected) {
                $client.EndConnect($connect)
                $ready = $true
                break
            }
        } catch {
            # The server may still be starting.
        } finally {
            $client.Dispose()
        }
        Start-Sleep -Milliseconds 100
    }

    if (-not $ready) {
        throw "Timed out waiting for the HTTP server on port $Port."
    }

    Write-Host "Brookesia layout test: $url"
    Write-Host "Press Ctrl+C to stop the server."
    if (-not $NoBrowser) {
        Start-Process $url
    }

    Wait-Process -Id $server.Id
} finally {
    if (-not $server.HasExited) {
        Stop-Process -Id $server.Id
    }
}
