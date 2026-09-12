[CmdletBinding()]
param(
    [ValidateRange(1, 65535)]
    [int]$Port = 8000,
    [switch]$NoBrowser
)

$ErrorActionPreference = "Stop"
$buildDirectory = Join-Path $PSScriptRoot "build"
$outputDirectory = Join-Path $PSScriptRoot "output"

if (-not (Test-Path -LiteralPath $buildDirectory -PathType Container)) {
    throw "The build directory was not found. Run a build script first."
}
if (-not (Test-Path -LiteralPath $outputDirectory -PathType Container)) {
    New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
}

$browserBuilds = @(
    Get-ChildItem -LiteralPath $buildDirectory -Filter "*.html" -File -Recurse |
        ForEach-Object {
            $baseName = [System.IO.Path]::GetFileNameWithoutExtension($_.Name)
            $directory = $_.DirectoryName
            $javascript = Join-Path $directory "$baseName.js"
            $wasm = Join-Path $directory "$baseName.wasm"
            if ((Test-Path -LiteralPath $javascript -PathType Leaf) -and
                (Test-Path -LiteralPath $wasm -PathType Leaf)) {
                [pscustomobject]@{
                    Name = $baseName
                    Html = $_.FullName
                    JavaScript = $javascript
                    Wasm = $wasm
                    Data = Join-Path $directory "$baseName.data"
                }
            }
        }
)

if ($browserBuilds.Count -eq 0) {
    throw "No HTML, JavaScript, and WebAssembly build triplets were found under $buildDirectory."
}

$duplicateNames = $browserBuilds | Group-Object Name | Where-Object Count -gt 1
if ($duplicateNames) {
    $names = ($duplicateNames | ForEach-Object Name) -join ", "
    throw "Build output names must be unique before staging: $names"
}

foreach ($build in $browserBuilds) {
    Copy-Item -LiteralPath $build.Html, $build.JavaScript, $build.Wasm -Destination $outputDirectory -Force
    if (Test-Path -LiteralPath $build.Data -PathType Leaf) {
        Copy-Item -LiteralPath $build.Data -Destination $outputDirectory -Force
    }
}

$links = $browserBuilds |
    Sort-Object Name |
    ForEach-Object {
        $name = [System.Net.WebUtility]::HtmlEncode($_.Name)
        "    <li><a href=`"$name.html`">$name</a></li>"
    }
$index = @"
<!doctype html>
<html lang="en">
<head><meta charset="utf-8"><title>Brookesia WASM builds</title></head>
<body>
  <h1>Brookesia WASM builds</h1>
  <ul>
$($links -join [Environment]::NewLine)
  </ul>
</body>
</html>
"@
[System.IO.File]::WriteAllText((Join-Path $outputDirectory "index.html"), $index, [System.Text.UTF8Encoding]::new($false))

$python = Get-Command python -ErrorAction SilentlyContinue
$pythonArguments = @("-m", "http.server", $Port, "--bind", "127.0.0.1")
if ($null -eq $python) {
    $python = Get-Command py -ErrorAction SilentlyContinue
    $pythonArguments = @("-3") + $pythonArguments
}
if ($null -eq $python) {
    throw "Python was not found on PATH. Install Python or add it to PATH."
}

$url = "http://localhost:$Port/index.html"
$server = Start-Process `
    -FilePath $python.Source `
    -ArgumentList $pythonArguments `
    -WorkingDirectory $outputDirectory `
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

    Write-Host "Brookesia WASM builds: $url"
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
