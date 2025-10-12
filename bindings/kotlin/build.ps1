# Build script for UMICP Kotlin SDK (PowerShell)
# Builds, tests, and packages the Kotlin library

$ErrorActionPreference = "Stop"

Write-Host "Building UMICP Kotlin SDK..." -ForegroundColor Cyan

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Check if Java is installed
if (-not (Get-Command java -ErrorAction SilentlyContinue)) {
    Write-Host "Java not found. Please install Java 17 or higher." -ForegroundColor Red
    exit 1
}

# Check Java version
Write-Host "Checking Java version..." -ForegroundColor Blue
$JavaVersion = java -version 2>&1 | Select-String -Pattern 'version' | Select-Object -First 1
Write-Host "Java version: $JavaVersion"

# Clean previous builds
Write-Host "Cleaning previous builds..." -ForegroundColor Blue
.\gradlew.bat clean

# Run tests
Write-Host "Running tests..." -ForegroundColor Blue
.\gradlew.bat test --no-daemon
if ($LASTEXITCODE -ne 0) {
    Write-Host "Tests failed! Fix errors before building." -ForegroundColor Red
    exit 1
}

# Generate test report
Write-Host "Generating coverage report..." -ForegroundColor Blue
.\gradlew.bat jacocoTestReport --no-daemon

# Check code quality
Write-Host "Checking code quality..." -ForegroundColor Blue
.\gradlew.bat check --no-daemon
if ($LASTEXITCODE -ne 0) {
    Write-Host "Code quality warnings (non-blocking)" -ForegroundColor Yellow
}

# Build the project
Write-Host "Building project..." -ForegroundColor Blue
.\gradlew.bat build --no-daemon

# Generate documentation
Write-Host "Generating documentation..." -ForegroundColor Blue
.\gradlew.bat dokkaHtml --no-daemon
if ($LASTEXITCODE -ne 0) {
    Write-Host "Documentation generation skipped" -ForegroundColor Yellow
}

# Get version from build.gradle.kts
$BuildGradle = Get-Content build.gradle.kts
$Version = ($BuildGradle | Select-String -Pattern 'version = "(.*?)"').Matches.Groups[1].Value

# Success message
Write-Host ""
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "Package: com.hivellm:umicp-kotlin"
Write-Host "Version: $Version"
Write-Host ""
Write-Host "Generated artifacts:"
Get-ChildItem -Path "build\libs\*.jar" -ErrorAction SilentlyContinue | Format-Table Name, Length
Write-Host ""
Write-Host "Test report: build\reports\tests\test\index.html"
Write-Host "Coverage report: build\reports\jacoco\test\html\index.html"
Write-Host "Documentation: build\docs\index.html"
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Test locally: .\gradlew.bat publishToMavenLocal"
Write-Host "  2. Publish to Maven Central: .\publish.ps1"
Write-Host ""

