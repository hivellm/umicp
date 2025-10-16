# Build script for UMICP Swift bindings (Windows/PowerShell)

$ErrorActionPreference = "Stop"

Write-Host "🔨 Building UMICP Swift..." -ForegroundColor Green

# Check Swift version
swift --version

# Clean previous build
Write-Host "🧹 Cleaning previous build..." -ForegroundColor Yellow
swift package clean

# Build for release
Write-Host "📦 Building release..." -ForegroundColor Cyan
swift build -c release

# Run tests
Write-Host "🧪 Running tests..." -ForegroundColor Magenta
swift test

Write-Host "✅ Build completed successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "📊 Build artifacts:" -ForegroundColor White
swift build --show-bin-path

Write-Host ""
Write-Host "🚀 To run examples:" -ForegroundColor White
Write-Host "  swift run BasicEnvelope"
Write-Host "  swift run ToolDiscovery"
Write-Host "  swift run MatrixOperations"

