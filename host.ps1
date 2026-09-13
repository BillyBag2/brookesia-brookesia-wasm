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
                    StagedName = $baseName
                    BuildDirectory = $directory
                    Html = $_.FullName
                    JavaScript = $javascript
                    Wasm = $wasm
                    Data = Join-Path $directory "$baseName.data"
                    Host = Join-Path $directory "$baseName-host.html"
                }
            }
        }
)

if ($browserBuilds.Count -eq 0) {
    throw "No HTML, JavaScript, and WebAssembly build triplets were found under $buildDirectory."
}

# Keep a flat output directory while retaining builds that happen to use the
# same executable name. The conventional build directory (for example
# build/bb-superos) keeps that name; other copies use their directory name.
$duplicateNames = $browserBuilds | Group-Object Name | Where-Object Count -gt 1
foreach ($duplicate in $duplicateNames) {
    foreach ($build in $duplicate.Group) {
        $buildDirectoryName = Split-Path -Leaf $build.BuildDirectory
        if ($buildDirectoryName -ne $build.Name) {
            $build.StagedName = $buildDirectoryName
        }
    }
}

$duplicateStagedNames = $browserBuilds | Group-Object StagedName | Where-Object Count -gt 1
if ($duplicateStagedNames) {
    $names = ($duplicateStagedNames | ForEach-Object Name) -join ", "
    throw "Build directories must have unique names before staging: $names"
}

foreach ($build in $browserBuilds) {
    # Keep each build's original filenames together. Emscripten's generated
    # JavaScript may refer to its matching .wasm or .data filename directly.
    $stageDirectory = Join-Path $outputDirectory $build.StagedName
    if (-not (Test-Path -LiteralPath $stageDirectory -PathType Container)) {
        New-Item -ItemType Directory -Path $stageDirectory -Force | Out-Null
    }
    Copy-Item -LiteralPath $build.Html, $build.JavaScript, $build.Wasm -Destination $stageDirectory -Force
    if (Test-Path -LiteralPath $build.Data -PathType Leaf) {
        Copy-Item -LiteralPath $build.Data -Destination $stageDirectory -Force
    }
    if (Test-Path -LiteralPath $build.Host -PathType Leaf) {
        Copy-Item -LiteralPath $build.Host -Destination $stageDirectory -Force
    }
}

$links = $browserBuilds |
    Sort-Object Name |
    ForEach-Object {
        $name = [System.Net.WebUtility]::HtmlEncode($_.StagedName)
        # Windows PowerShell 5.1 runs on .NET Framework, which does not expose
        # System.IO.Path.GetRelativePath(). Every discovered build is below
        # $buildDirectory, so trim that known prefix instead.
        $buildPath = $_.BuildDirectory.Substring($buildDirectory.Length).TrimStart('\', '/')
        $buildPath = [System.Net.WebUtility]::HtmlEncode($buildPath.Replace('\', '/'))
        $hostName = if (Test-Path -LiteralPath $_.Host -PathType Leaf) {
            "$($_.Name)-host.html"
        } else {
            "$($_.Name).html"
        }
        $htmlName = [System.Net.WebUtility]::HtmlEncode($hostName)
        "    <li><a href=`"$name/$htmlName`">$name</a> <small>($buildPath)</small></li>"
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
Write-Host "Staged browser builds: $(($browserBuilds | Sort-Object StagedName | ForEach-Object StagedName) -join ', ')"

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
