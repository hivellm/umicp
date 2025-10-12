#!/bin/bash
# Build script for UMICP C# SDK
# This script builds and packages the NuGet package

set -e

echo "🔨 Building UMICP C# SDK..."

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if dotnet is installed
if ! command -v dotnet &> /dev/null; then
    echo -e "${RED}❌ .NET SDK not found. Please install .NET 8.0 SDK.${NC}"
    echo "Download from: https://dotnet.microsoft.com/download"
    exit 1
fi

# Check .NET version
echo -e "${BLUE}📋 Checking .NET version...${NC}"
DOTNET_VERSION=$(dotnet --version)
echo ".NET SDK version: $DOTNET_VERSION"

# Clean previous builds
echo -e "${BLUE}🧹 Cleaning previous builds...${NC}"
rm -rf Umicp.Core/bin/Release
rm -rf Umicp.Core/obj/Release
rm -rf Umicp.Core/obj/project.assets.json
rm -rf artifacts/*.nupkg artifacts/*.snupkg 2>/dev/null || true
mkdir -p artifacts

# Clear NuGet cache to avoid Windows path issues
echo -e "${BLUE}🔧 Clearing NuGet cache...${NC}"
dotnet nuget locals all --clear > /dev/null 2>&1 || true

# Restore dependencies (Core project only)
echo -e "${BLUE}📦 Restoring dependencies...${NC}"
dotnet restore Umicp.Core/Umicp.Core.csproj

# Build the project
echo -e "${BLUE}🔨 Building project...${NC}"
dotnet build Umicp.Core/Umicp.Core.csproj --configuration Release --no-restore

# Pack NuGet package
echo -e "${BLUE}📦 Creating NuGet package...${NC}"
dotnet pack Umicp.Core/Umicp.Core.csproj \
    --configuration Release \
    --no-build \
    --output artifacts \
    --include-symbols \
    --include-source

# Display results
echo ""
echo -e "${GREEN}✅ Build completed successfully!${NC}"
echo ""
echo "Generated artifacts:"
ls -lh artifacts/*.nupkg 2>/dev/null || echo "No packages found"
ls -lh artifacts/*.snupkg 2>/dev/null || echo "No symbol packages found"

# Get package version
VERSION=$(grep -oP '<Version>\K[^<]+' Umicp.Core/Umicp.Core.csproj)

echo ""
echo "Package version: $VERSION"
echo ""
echo "Next steps:"
echo "  1. Test the package: dotnet add package HiveLLM.Umicp --source ./artifacts"
echo "  2. Upload to NuGet: ./publish.sh"
echo ""

