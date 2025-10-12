# Build script for UMICP PHP SDK (PowerShell)
# Validates and prepares the package for Packagist/Composer

$ErrorActionPreference = "Stop"

Write-Host "Building UMICP PHP SDK..." -ForegroundColor Cyan

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Check if PHP is installed
if (-not (Get-Command php -ErrorAction SilentlyContinue)) {
    Write-Host "PHP not found. Please install PHP 8.1 or higher." -ForegroundColor Red
    exit 1
}

# Check PHP version
Write-Host "Checking PHP version..." -ForegroundColor Blue
$PhpVersion = php -r "echo PHP_VERSION;"
Write-Host "PHP version: $PhpVersion"

# Check required extensions
Write-Host "Checking required extensions..." -ForegroundColor Blue
$RequiredExts = @("ffi", "json")
$PhpModules = php -m

foreach ($ext in $RequiredExts) {
    if ($PhpModules -match $ext) {
        Write-Host "  $ext" -ForegroundColor Green
    } else {
        Write-Host "Required extension '$ext' not found." -ForegroundColor Red
        exit 1
    }
}

# Check if composer is installed
if (-not (Get-Command composer -ErrorAction SilentlyContinue)) {
    Write-Host "Composer not found. Please install Composer." -ForegroundColor Red
    Write-Host "Download from: https://getcomposer.org/download/"
    exit 1
}

# Validate composer.json
Write-Host "Validating composer.json..." -ForegroundColor Blue
composer validate --strict
if ($LASTEXITCODE -ne 0) {
    Write-Host "composer.json validation failed!" -ForegroundColor Red
    exit 1
}

# Install dependencies
Write-Host "Installing dependencies..." -ForegroundColor Blue
composer install --no-dev --optimize-autoloader

# Install dev dependencies for testing
Write-Host "Installing dev dependencies..." -ForegroundColor Blue
composer install --optimize-autoloader

# Run linting
Write-Host "Running code style check (PSR-12)..." -ForegroundColor Blue
.\vendor\bin\phpcs --standard=PSR12 src\ --report=summary
if ($LASTEXITCODE -ne 0) {
    Write-Host "Linting warnings found (non-blocking)" -ForegroundColor Yellow
}

# Run static analysis
Write-Host "Running static analysis (PHPStan)..." -ForegroundColor Blue
.\vendor\bin\phpstan analyse src\ --level=8 --no-progress
if ($LASTEXITCODE -ne 0) {
    Write-Host "Static analysis warnings found (non-blocking)" -ForegroundColor Yellow
}

# Note: Tests require compiled FFI library (libumicp_core.so)
# For Packagist publication, we only validate the package structure
Write-Host "Skipping FFI-dependent tests (library not compiled)" -ForegroundColor Yellow
Write-Host "For full test suite, compile C++ library first: .\build-cpp.ps1" -ForegroundColor Blue

# Run validation
Write-Host "Validating package structure..." -ForegroundColor Blue
php validate-package.php
if ($LASTEXITCODE -ne 0) {
    Write-Host "Package validation failed!" -ForegroundColor Red
    exit 1
}

# Get package version from Git tags or prompt
try {
    $LatestTag = git describe --tags --abbrev=0 2>$null
    $Version = $LatestTag -replace '^v', ''
} catch {
    $Version = "dev-main"
}

# Success message
Write-Host ""
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "Package: hivellm/umicp"
Write-Host "Version: $Version (from Git tags)"
Write-Host ""
Write-Host "Package is ready for Packagist!"
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Commit changes: git add . && git commit -m 'Release v$Version'"
Write-Host "  2. Create tag: git tag v$Version"
Write-Host "  3. Push with tags: git push origin main --tags"
Write-Host "  4. Packagist will auto-update (if webhook configured)"
Write-Host ""
Write-Host "Manual publish: .\publish.ps1"
Write-Host ""

