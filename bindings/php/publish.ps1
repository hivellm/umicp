# Publish script for UMICP PHP SDK to Packagist (PowerShell)
# Packagist automatically pulls from Git tags, so this script creates and pushes a tag

param(
    [string]$Version
)

$ErrorActionPreference = "Stop"

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

Write-Host "Publishing UMICP PHP SDK to Packagist..." -ForegroundColor Blue

# Check if git is installed
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "Git not found." -ForegroundColor Red
    exit 1
}

# Get package version from argument or prompt
if (-not $Version) {
    Write-Host "No version provided as argument" -ForegroundColor Yellow
    $Version = Read-Host "Enter version to release (e.g., 0.1.3)"
}

if (-not $Version) {
    Write-Host "Version is required" -ForegroundColor Red
    exit 1
}

# Remove 'v' prefix if present
$Version = $Version -replace '^v', ''

Write-Host "Package: hivellm/umicp"
Write-Host "Version: $Version"
Write-Host ""

# Check if there are uncommitted changes
$Status = git status --porcelain
if ($Status) {
    Write-Host "You have uncommitted changes!" -ForegroundColor Yellow
    Write-Host ""
    git status --short
    Write-Host ""
    $commitConfirm = Read-Host "Commit changes before tagging? (yes/no)"
    if ($commitConfirm -eq "yes") {
        Write-Host ""
        $commitMsg = Read-Host "Commit message"
        git add .
        git commit -m $commitMsg
        Write-Host "Changes committed" -ForegroundColor Green
    } else {
        Write-Host "Proceeding with uncommitted changes..." -ForegroundColor Yellow
    }
}

# Check if tag already exists
$TagExists = git tag -l "v$Version"
if ($TagExists) {
    Write-Host "Tag v$Version already exists!" -ForegroundColor Yellow
    $tagConfirm = Read-Host "Delete and recreate tag? (yes/no)"
    if ($tagConfirm -eq "yes") {
        git tag -d "v$Version"
        git push origin ":refs/tags/v$Version" 2>$null
        Write-Host "Old tag deleted" -ForegroundColor Green
    } else {
        Write-Host "Cannot proceed with existing tag" -ForegroundColor Red
        exit 1
    }
}

# Create annotated tag
Write-Host "Creating tag v$Version..." -ForegroundColor Blue
$tagMessage = @"
Release v$Version

- UMICP PHP SDK production release
- Full BIP-05 protocol compliance
- 115+ tests with 95% coverage
- WebSocket transport (client/server)
- Multiplexed peer architecture
- Service Discovery & Connection Pooling
- Compression support (GZIP/DEFLATE)
- Production ready

See docs/CHANGELOG.md for details.
"@

git tag -a "v$Version" -m $tagMessage
Write-Host "Tag created" -ForegroundColor Green

# Confirm push (unless in CI)
if (-not $env:CI) {
    Write-Host ""
    Write-Host "Ready to push to remote repository!" -ForegroundColor Yellow
    Write-Host "This will trigger Packagist auto-update (if configured)"
    Write-Host ""
    $pushConfirm = Read-Host "Push tag v$Version to origin? (yes/no)"
    if ($pushConfirm -ne "yes") {
        Write-Host "Push cancelled. Tag created locally only."
        Write-Host "To push later: git push origin v$Version"
        exit 0
    }
}

# Push tag to remote
Write-Host "Pushing tag to origin..." -ForegroundColor Blue
git push origin "v$Version"

Write-Host ""
Write-Host "Tag pushed successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "Packagist Publication:" -ForegroundColor Cyan
Write-Host ""
Write-Host "If your package is already on Packagist with GitHub webhook:"
Write-Host "  -> Packagist will auto-update within minutes"
Write-Host "  -> View at: https://packagist.org/packages/hivellm/umicp"
Write-Host ""
Write-Host "If this is your first release:"
Write-Host "  1. Go to: https://packagist.org/packages/submit"
Write-Host "  2. Submit: https://github.com/hivellm/umicp"
Write-Host "  3. Configure GitHub Service Hook for auto-updates"
Write-Host ""
Write-Host "Installation:"
Write-Host "  composer require hivellm/umicp"
Write-Host ""
