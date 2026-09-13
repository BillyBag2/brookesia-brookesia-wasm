[CmdletBinding()]
param(
    [switch]$Clean,
    [string]$LockFile = "wasm-components.lock.json"
)

$ErrorActionPreference = "Stop"

$repositoryRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$dependencyRoot = [System.IO.Path]::GetFullPath((Join-Path $repositoryRoot ".deps"))
$assembledRoot = Join-Path $dependencyRoot "assembled"

function Assert-GeneratedPath {
    param([Parameter(Mandatory = $true)][string]$Path)

    $fullPath = [System.IO.Path]::GetFullPath($Path)
    $requiredPrefix = $repositoryRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar) +
        [System.IO.Path]::DirectorySeparatorChar + ".deps" +
        [System.IO.Path]::DirectorySeparatorChar

    if (($fullPath + [System.IO.Path]::DirectorySeparatorChar) -eq $requiredPrefix) {
        return
    }
    if (-not $fullPath.StartsWith($requiredPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to modify a path outside the generated .deps directory: $fullPath"
    }
}

function Remove-GeneratedDirectory {
    param([Parameter(Mandatory = $true)][string]$Path)

    Assert-GeneratedPath -Path $Path
    if (Test-Path -LiteralPath $Path) {
        Remove-Item -LiteralPath $Path -Recurse -Force
    }
}

function New-Directory {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

function Join-WebUrl {
    param(
        [Parameter(Mandatory = $true)][string]$Base,
        [Parameter(Mandatory = $true)][string]$Relative
    )

    return [System.Uri]::new([System.Uri]$Base, $Relative).AbsoluteUri
}

function Download-File {
    param(
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][string]$Destination
    )

    if (Test-Path -LiteralPath $Destination) {
        return
    }

    New-Directory -Path (Split-Path -Parent $Destination)
    Write-Host "Downloading $Url"
    Invoke-WebRequest -Uri $Url -OutFile $Destination -UseBasicParsing
}

function Test-ComponentChecksums {
    param(
        [Parameter(Mandatory = $true)][string]$ComponentDirectory,
        [Parameter(Mandatory = $true)][string]$ChecksumsFile
    )

    $manifest = Get-Content -LiteralPath $ChecksumsFile -Raw | ConvertFrom-Json
    if ($manifest.algorithm -ne "sha256") {
        throw "Unsupported checksum algorithm '$($manifest.algorithm)' in $ChecksumsFile"
    }

    foreach ($file in $manifest.files) {
        $relativePath = [string]$file.path
        $candidate = [System.IO.Path]::GetFullPath((Join-Path $ComponentDirectory $relativePath))
        $componentPrefix = [System.IO.Path]::GetFullPath($ComponentDirectory).TrimEnd(
            [System.IO.Path]::DirectorySeparatorChar
        ) + [System.IO.Path]::DirectorySeparatorChar

        if (-not $candidate.StartsWith($componentPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Checksum manifest contains a path outside the component: $relativePath"
        }
        if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            throw "Component file is missing: $relativePath"
        }

        $actualHash = (Get-FileHash -LiteralPath $candidate -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actualHash -ne ([string]$file.hash).ToLowerInvariant()) {
            throw "Checksum mismatch for $relativePath"
        }
    }
}

function Install-AssembledDirectory {
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [Parameter(Mandatory = $true)][string]$RelativeDestination
    )

    $destination = [System.IO.Path]::GetFullPath((Join-Path $assembledRoot $RelativeDestination))
    Assert-GeneratedPath -Path $destination

    if (Test-Path -LiteralPath $destination) {
        Write-Host "Using existing assembled directory: $RelativeDestination"
        return
    }

    New-Directory -Path (Split-Path -Parent $destination)
    Copy-Item -LiteralPath $Source -Destination $destination -Recurse
    Write-Host "Assembled $RelativeDestination"
}

function Get-RegistryComponent {
    param(
        [Parameter(Mandatory = $true)]$Entry,
        [Parameter(Mandatory = $true)][string]$StorageUrl
    )

    $folderName = ([string]$Entry.name).Replace("/", "__")
    $sourceDirectory = Join-Path $dependencyRoot "sources/registry/$($Entry.group)/$folderName"

    if (Test-Path -LiteralPath $sourceDirectory) {
        Write-Host "Using existing Registry component: $($Entry.name)@$($Entry.version)"
        return $sourceDirectory
    }

    $metadataUrl = Join-WebUrl -Base $StorageUrl -Relative "components/$($Entry.name).json"
    Write-Host "Reading Registry metadata for $($Entry.name)@$($Entry.version)"
    $metadata = Invoke-RestMethod -Uri $metadataUrl
    $versionInfo = @($metadata.versions | Where-Object { $_.version -eq $Entry.version })
    if ($versionInfo.Count -ne 1) {
        throw "Registry version $($Entry.name)@$($Entry.version) was not found uniquely"
    }
    $versionInfo = $versionInfo[0]

    if (([string]$versionInfo.component_hash).ToLowerInvariant() -ne
        ([string]$Entry.componentHash).ToLowerInvariant()) {
        throw "Registry component hash changed for $($Entry.name)@$($Entry.version)"
    }

    $archiveName = "$folderName-v$($Entry.version).zip"
    $cacheDirectory = Join-Path $dependencyRoot "cache/registry/$folderName/$($Entry.version)"
    $archivePath = Join-Path $cacheDirectory $archiveName
    $checksumsPath = Join-Path $cacheDirectory "CHECKSUMS.json"
    Download-File -Url (Join-WebUrl -Base $StorageUrl -Relative $versionInfo.url) -Destination $archivePath
    Download-File -Url (Join-WebUrl -Base $StorageUrl -Relative $versionInfo.checksums) -Destination $checksumsPath

    $stagingDirectory = Join-Path $dependencyRoot ("cache/staging/" + [guid]::NewGuid().ToString("N"))
    Assert-GeneratedPath -Path $stagingDirectory
    New-Directory -Path $stagingDirectory

    try {
        Expand-Archive -LiteralPath $archivePath -DestinationPath $stagingDirectory
        Test-ComponentChecksums -ComponentDirectory $stagingDirectory -ChecksumsFile $checksumsPath
        Copy-Item -LiteralPath $checksumsPath -Destination (Join-Path $stagingDirectory "CHECKSUMS.json") -Force
        Set-Content -LiteralPath (Join-Path $stagingDirectory ".component_hash") `
            -Value $Entry.componentHash -NoNewline -Encoding Ascii

        New-Directory -Path (Split-Path -Parent $sourceDirectory)
        Move-Item -LiteralPath $stagingDirectory -Destination $sourceDirectory
    }
    finally {
        if (Test-Path -LiteralPath $stagingDirectory) {
            Remove-GeneratedDirectory -Path $stagingDirectory
        }
    }

    Write-Host "Installed Registry component: $($Entry.name)@$($Entry.version)"
    return $sourceDirectory
}

function Invoke-Git {
    $gitArguments = @($args)
    $previousErrorActionPreference = $ErrorActionPreference
    try {
        # Windows PowerShell 5 turns native stderr, including Git progress, into
        # ErrorRecord objects. The native exit code remains the success check.
        $ErrorActionPreference = "Continue"
        $gitOutput = & git @gitArguments 2>&1
        $gitExitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    foreach ($line in $gitOutput) {
        Write-Host $line
    }
    if ($gitExitCode -ne 0) {
        throw "Git failed with exit code ${gitExitCode}: git $($gitArguments -join ' ')"
    }
}

function Get-GitSource {
    param([Parameter(Mandatory = $true)]$Entry)

    $shortCommit = ([string]$Entry.commit).Substring(0, 12)
    $sourceName = ([string]$Entry.name) -replace '[^A-Za-z0-9._-]', '-'
    $checkoutDirectory = Join-Path $dependencyRoot "sources/git/$sourceName-$shortCommit"
    $componentDirectory = Join-Path $checkoutDirectory $Entry.path

    if (Test-Path -LiteralPath $checkoutDirectory) {
        if (-not (Test-Path -LiteralPath $componentDirectory -PathType Container)) {
            throw "Existing Git source is incomplete. Run this script with -Clean."
        }
        Write-Host "Using existing Git source: $($Entry.name)@$shortCommit"
        return $componentDirectory
    }

    New-Directory -Path (Split-Path -Parent $checkoutDirectory)
    Invoke-Git init $checkoutDirectory
    Invoke-Git -C $checkoutDirectory remote add origin $Entry.repository
    Invoke-Git -C $checkoutDirectory sparse-checkout init --cone
    Invoke-Git -C $checkoutDirectory sparse-checkout set $Entry.path
    Invoke-Git -C $checkoutDirectory fetch --depth 1 origin $Entry.commit
    Invoke-Git -C $checkoutDirectory checkout --detach FETCH_HEAD

    if (-not (Test-Path -LiteralPath $componentDirectory -PathType Container)) {
        throw "Git source path was not found after checkout: $($Entry.path)"
    }

    Write-Host "Installed Git source: $($Entry.name)@$shortCommit"
    return $componentDirectory
}

if ([System.IO.Path]::IsPathRooted($LockFile)) {
    $resolvedLockFile = [System.IO.Path]::GetFullPath($LockFile)
}
else {
    $resolvedLockFile = [System.IO.Path]::GetFullPath((Join-Path $repositoryRoot $LockFile))
}

if (-not (Test-Path -LiteralPath $resolvedLockFile -PathType Leaf)) {
    throw "Component lock file was not found: $resolvedLockFile"
}

$lock = Get-Content -LiteralPath $resolvedLockFile -Raw | ConvertFrom-Json
if ($lock.schemaVersion -ne 1) {
    throw "Unsupported component lock schema version: $($lock.schemaVersion)"
}

if ($Clean) {
    Write-Host "Removing generated dependencies: $dependencyRoot"
    Remove-GeneratedDirectory -Path $dependencyRoot
}

New-Directory -Path $dependencyRoot

foreach ($entry in $lock.registry) {
    $source = Get-RegistryComponent -Entry $entry -StorageUrl $lock.registryStorageUrl
    Install-AssembledDirectory -Source $source -RelativeDestination $entry.destination
}

foreach ($entry in $lock.git) {
    $source = Get-GitSource -Entry $entry
    Install-AssembledDirectory -Source $source -RelativeDestination $entry.destination
}

Write-Host "WASM source tree is ready at $assembledRoot"
