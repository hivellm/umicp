# Build C++ core library for PHP FFI (Windows PowerShell)
# This script builds the UMICP C++ core as a shared library

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  UMICP C++ Core - Build for PHP FFI" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Navigate to C++ directory
Set-Location "..\..\cpp"

# Create build directory
Write-Host "1. Creating build directory..." -ForegroundColor Yellow
New-Item -ItemType Directory -Force -Path "build" | Out-Null
Set-Location "build"

# Configure CMake
Write-Host ""
Write-Host "2. Configuring CMake..." -ForegroundColor Yellow
cmake .. `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_SHARED_LIBS=ON `
    -DBUILD_EXAMPLES=OFF `
    -DBUILD_TESTS=OFF

# Build
Write-Host ""
Write-Host "3. Building..." -ForegroundColor Yellow
cmake --build . --config Release --target umicp_core

# Check if library was built
$libraryBuilt = $false

if (Test-Path "Release\umicp_core.dll") {
    $libraryBuilt = $true
    $libraryPath = "Release\umicp_core.dll"
} elseif (Test-Path "libumicp_core.so") {
    $libraryBuilt = $true
    $libraryPath = "libumicp_core.so"
} elseif (Test-Path "libumicp_core.dylib") {
    $libraryBuilt = $true
    $libraryPath = "libumicp_core.dylib"
}

if ($libraryBuilt) {
    Write-Host ""
    Write-Host "✓ Build successful!" -ForegroundColor Green
    Write-Host ""

    Write-Host "Library: $libraryPath" -ForegroundColor Cyan
    Get-Item $libraryPath | Format-List Length, LastWriteTime

    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  1. Update config/umicp.php with library path"
    Write-Host "  2. Run: composer install"
    Write-Host "  3. Test: php examples/01_basic_envelope.php"
} else {
    Write-Host ""
    Write-Host "✗ Build failed!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Check build output above for errors."
    exit 1
}

