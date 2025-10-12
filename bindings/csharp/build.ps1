# Build script for UMICP C# SDK (PowerShell)
# This script builds and packages the NuGet package

$ErrorActionPreference = "Stop"

Write-Host "🔨 Building UMICP C# SDK..." -ForegroundColor Cyan

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# Check if dotnet is installed
if (-not (Get-Command dotnet -ErrorAction SilentlyContinue)) {
    Write-Host "❌ .NET SDK not found. Please install .NET 8.0 SDK." -ForegroundColor Red
    Write-Host "Download from: https://dotnet.microsoft.com/download"
    exit 1
}

# Check .NET version
Write-Host "📋 Checking .NET version..." -ForegroundColor Blue
$DotnetVersion = dotnet --version
Write-Host ".NET SDK version: $DotnetVersion"

# Clean previous builds
Write-Host "🧹 Cleaning previous builds..." -ForegroundColor Blue
dotnet clean Umicp.sln --configuration Release
Remove-Item -Path "Umicp.Core\bin\Release" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "Umicp.Core\obj\Release" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "artifacts\*.nupkg" -Force -ErrorAction SilentlyContinue
Remove-Item -Path "artifacts\*.snupkg" -Force -ErrorAction SilentlyContinue

# Restore dependencies
Write-Host "📦 Restoring dependencies..." -ForegroundColor Blue
dotnet restore Umicp.sln

# Run tests before building
Write-Host "🧪 Running tests..." -ForegroundColor Blue
dotnet test Umicp.Tests\Umicp.Tests.csproj --configuration Release --verbosity minimal --no-restore
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Tests failed! Fix errors before building." -ForegroundColor Red
    exit 1
}

# Build the project
Write-Host "🔨 Building project..." -ForegroundColor Blue
dotnet build Umicp.Core\Umicp.Core.csproj --configuration Release --no-restore

# Pack NuGet package
Write-Host "📦 Creating NuGet package..." -ForegroundColor Blue
dotnet pack Umicp.Core\Umicp.Core.csproj `
    --configuration Release `
    --no-build `
    --output artifacts `
    --include-symbols `
    --include-source

# Display results
Write-Host ""
Write-Host "✅ Build completed successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "Generated artifacts:"
Get-ChildItem -Path "artifacts" -Filter "*.nupkg" -ErrorAction SilentlyContinue | Format-Table Name, Length, LastWriteTime
Get-ChildItem -Path "artifacts" -Filter "*.snupkg" -ErrorAction SilentlyContinue | Format-Table Name, Length, LastWriteTime

# Get package version
$CsprojContent = Get-Content "Umicp.Core\Umicp.Core.csproj"
$Version = ($CsprojContent | Select-String -Pattern '<Version>(.*?)</Version>').Matches.Groups[1].Value

Write-Host ""
Write-Host "Package version: $Version"
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Test the package: dotnet add package HiveLLM.Umicp --source .\artifacts"
Write-Host "  2. Upload to NuGet: .\publish.ps1"
Write-Host ""

