<#
.SYNOPSIS
    Fetches the DisplayXR Unreal plugin from GitHub Releases into Plugins/DisplayXR.

.DESCRIPTION
    Reads the version tag from .displayxr-version at the repo root, downloads the
    matching DisplayXR_5.7.zip asset from DisplayXR/displayxr-unreal, and extracts
    it to Plugins/DisplayXR, replacing any previous install.

    Requires: GitHub CLI (gh) authenticated via `gh auth login`. No token hard-coded.

.EXAMPLE
    pwsh Scripts/fetch-plugin.ps1
#>

$ErrorActionPreference = "Stop"

$repoRoot    = Resolve-Path "$PSScriptRoot\.."
$versionFile = Join-Path $repoRoot ".displayxr-version"
$tag         = (Get-Content $versionFile).Trim()
$asset       = "DisplayXR_5.7.zip"
$dest        = Join-Path $repoRoot "Plugins\DisplayXR"
$tmpDir      = Join-Path $env:TEMP "displayxr-fetch"
$tmpZip      = Join-Path $tmpDir $asset

if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
    throw "GitHub CLI (gh) not found on PATH. Install from https://cli.github.com/ and run 'gh auth login'."
}

Write-Host "Fetching DisplayXR $tag (asset $asset) from DisplayXR/displayxr-unreal..."

if (Test-Path $tmpDir) { Remove-Item -Recurse -Force $tmpDir }
New-Item -ItemType Directory -Path $tmpDir -Force | Out-Null

gh release download $tag `
    --repo DisplayXR/displayxr-unreal `
    --pattern $asset `
    --dir $tmpDir `
    --clobber
if ($LASTEXITCODE -ne 0) {
    throw "gh release download failed (exit $LASTEXITCODE). Is '$tag' a published release with a $asset asset?"
}

if (Test-Path $dest) {
    Write-Host "Removing previous install at $dest"
    Remove-Item -Recurse -Force $dest
}
New-Item -ItemType Directory -Path (Split-Path $dest) -Force | Out-Null

Write-Host "Extracting $asset to $dest"
Expand-Archive -Path $tmpZip -DestinationPath $dest -Force

Remove-Item -Recurse -Force $tmpDir

$uplugin = Join-Path $dest "DisplayXR.uplugin"
if (-not (Test-Path $uplugin)) {
    throw "Extraction produced no DisplayXR.uplugin at $uplugin - archive layout unexpected."
}

Write-Host ""
Write-Host "DisplayXR $tag installed at $dest"
Write-Host "Open DisplayXRTest.uproject in Unreal Engine 5.7 to use it."
