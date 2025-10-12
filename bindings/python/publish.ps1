# Publish script for UMICP Python package (PowerShell)
# Usage: .\publish.ps1 [-Test]

param(
    [switch]$Test
)

$ErrorActionPreference = "Stop"

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Check for test flag
if ($Test) {
    $RepoUrl = "https://test.pypi.org/legacy/"
    $RepoName = "testpypi"
    Write-Host "📤 Publishing to Test PyPI..." -ForegroundColor Yellow
} else {
    $RepoUrl = "https://upload.pypi.org/legacy/"
    $RepoName = "pypi"
    Write-Host "📤 Publishing to PyPI..." -ForegroundColor Blue
}

# Check if dist directory exists
if (-not (Test-Path "dist") -or (Get-ChildItem "dist").Count -eq 0) {
    Write-Host "❌ No distribution files found. Run .\build.ps1 first." -ForegroundColor Red
    exit 1
}

# Check if twine is installed
try {
    python -c "import twine" 2>$null
} catch {
    Write-Host "❌ 'twine' not found. Installing..." -ForegroundColor Red
    pip install twine
}

# Verify package before upload
Write-Host "✅ Verifying package..." -ForegroundColor Blue
twine check dist/*

# Display what will be uploaded
Write-Host ""
Write-Host "Files to upload:"
Get-ChildItem -Path "dist" | Format-Table Name, Length

# Confirm upload (unless in CI)
if (-not $env:CI) {
    if (-not $Test) {
        Write-Host "⚠️  You are about to upload to PRODUCTION PyPI!" -ForegroundColor Yellow
        $confirm = Read-Host "Are you sure you want to continue? (yes/no)"
        if ($confirm -ne "yes") {
            Write-Host "Upload cancelled."
            exit 0
        }
    }
}

# Upload to PyPI
Write-Host "📤 Uploading to $RepoName..." -ForegroundColor Blue
if ($Test) {
    twine upload --repository testpypi dist/*
} else {
    twine upload dist/*
}

# Success message
Write-Host ""
Write-Host "✅ Package uploaded successfully!" -ForegroundColor Green
Write-Host ""
if ($Test) {
    Write-Host "View on Test PyPI: https://test.pypi.org/project/umicp-python/"
    Write-Host ""
    Write-Host "Install from Test PyPI:"
    Write-Host "  pip install --index-url https://test.pypi.org/simple/ umicp-python"
} else {
    Write-Host "View on PyPI: https://pypi.org/project/umicp-python/"
    Write-Host ""
    Write-Host "Install from PyPI:"
    Write-Host "  pip install umicp-python"
}
Write-Host ""

