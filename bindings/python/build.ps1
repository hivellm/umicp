# Build script for UMICP Python package (PowerShell)
# This script creates distribution packages (wheel and source distribution)

$ErrorActionPreference = "Stop"

Write-Host "🔨 Building UMICP Python package..." -ForegroundColor Cyan

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Check Python version
Write-Host "📋 Checking Python version..." -ForegroundColor Blue
$PythonVersion = python --version
Write-Host "Python version: $PythonVersion"

# Check if required tools are installed
Write-Host "📦 Checking build tools..." -ForegroundColor Blue

try {
    python -c "import build" 2>$null
} catch {
    Write-Host "⚠️  'build' module not found. Installing..." -ForegroundColor Yellow
    pip install build
}

try {
    python -c "import twine" 2>$null
} catch {
    Write-Host "⚠️  'twine' module not found. Installing..." -ForegroundColor Yellow
    pip install twine
}

# Clean previous builds
Write-Host "🧹 Cleaning previous builds..." -ForegroundColor Blue
Remove-Item -Path "build", "dist", "*.egg-info" -Recurse -Force -ErrorAction SilentlyContinue

# Run tests before building
Write-Host "🧪 Running tests..." -ForegroundColor Blue
$pytestExists = Get-Command pytest -ErrorAction SilentlyContinue

if ($pytestExists) {
    pytest tests/ -v
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Tests failed! Fix errors before building." -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "⚠️  pytest not found. Skipping tests." -ForegroundColor Yellow
}

# Build package
Write-Host "📦 Building package..." -ForegroundColor Blue
python -m build

# Verify package
Write-Host "✅ Verifying package..." -ForegroundColor Blue
twine check dist/*

# Display results
Write-Host ""
Write-Host "✅ Build completed successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "Generated artifacts:"
Get-ChildItem -Path "dist" | Format-Table Name, Length, LastWriteTime

Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Test the package: pip install dist\umicp_python-*.whl"
Write-Host "  2. Upload to Test PyPI: .\publish.ps1 -Test"
Write-Host "  3. Upload to PyPI: .\publish.ps1"
Write-Host ""

