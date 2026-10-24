# Builds the plugin and packages it as a .streamDeckPlugin file (Windows, Qt 6.8 + MinGW or MSVC)
#
#   .\tools\build-windows.ps1 -QtDir C:\Qt\6.8.3\mingw_64 [-MinGWDir C:\Qt\Tools\mingw1310_64] [-CMakeDir C:\Qt\Tools\CMake_64]
#
# The result is written to release\com.n3ckarmann.volumemixer.streamDeckPlugin.

param(
    [Parameter(Mandatory = $true)][string]$QtDir,
    [string]$MinGWDir,
    [string]$CMakeDir
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root "build"
$pluginDir = Join-Path $root "bin\Release\com.n3ckarmann.volumemixer.sdPlugin"
$releaseDir = Join-Path $root "release"

foreach ($dir in @($MinGWDir, $CMakeDir)) {
    if ($dir) { $env:PATH = (Join-Path $dir "bin") + ";" + $env:PATH }
}
$env:PATH = (Join-Path $QtDir "bin") + ";" + $env:PATH

function Invoke-Step([string]$name, [scriptblock]$cmd) {
    Write-Host "==> $name" -ForegroundColor Cyan
    & $cmd
    if ($LASTEXITCODE -ne 0) { throw "$name failed (exit code $LASTEXITCODE)" }
}

foreach ($dir in @($buildDir, $pluginDir, $releaseDir)) {
    if (Test-Path -LiteralPath $dir) { Remove-Item -LiteralPath $dir -Recurse -Force }
}

$generator = if ($MinGWDir -or (Get-Command mingw32-make -ErrorAction SilentlyContinue)) { "MinGW Makefiles" } else { "Visual Studio 17 2022" }

Invoke-Step "Configure ($generator)" { cmake -S $root -B $buildDir -G $generator -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$QtDir" }
Invoke-Step "Build" { cmake --build $buildDir --config Release --parallel }
Invoke-Step "Install + windeployqt" { cmake --install $buildDir --config Release }

# Stream Deck expects a zip with the .sdPlugin folder at its root.
# Compress-Archive (Windows PowerShell 5) writes backslashes into the entry names, so the zip is built by hand.
New-Item -ItemType Directory -Path $releaseDir | Out-Null
$target = Join-Path $releaseDir "com.n3ckarmann.volumemixer.streamDeckPlugin"
Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem
$archive = [System.IO.Compression.ZipFile]::Open($target, [System.IO.Compression.ZipArchiveMode]::Create)
try {
    $baseDir = Split-Path -Parent $pluginDir
    foreach ($file in Get-ChildItem -LiteralPath $pluginDir -Recurse -File) {
        $entryName = $file.FullName.Substring($baseDir.Length + 1).Replace('\', '/')
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, $file.FullName, $entryName) | Out-Null
    }
}
finally {
    $archive.Dispose()
}

Write-Host "Done: $target" -ForegroundColor Green
