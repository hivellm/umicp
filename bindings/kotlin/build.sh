#!/bin/bash
# Build script for UMICP Kotlin SDK
# Builds, tests, and packages the Kotlin library

set -e

echo "🔨 Building UMICP Kotlin SDK..."

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if Java is installed
if ! command -v java &> /dev/null; then
    echo -e "${RED}❌ Java not found. Please install Java 17 or higher.${NC}"
    exit 1
fi

# Check Java version
echo -e "${BLUE}📋 Checking Java version...${NC}"
JAVA_VERSION=$(java -version 2>&1 | head -n 1 | cut -d'"' -f2)
echo "Java version: $JAVA_VERSION"

# Clean previous builds
echo -e "${BLUE}🧹 Cleaning previous builds...${NC}"
./gradlew clean

# Run tests
echo -e "${BLUE}🧪 Running tests...${NC}"
./gradlew test --no-daemon
if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Tests failed! Fix errors before building.${NC}"
    exit 1
fi

# Generate test report
echo -e "${BLUE}📊 Generating coverage report...${NC}"
./gradlew jacocoTestReport --no-daemon

# Check code quality (if ktlint is configured)
echo -e "${BLUE}🔍 Checking code quality...${NC}"
./gradlew check --no-daemon || {
    echo -e "${YELLOW}⚠️  Code quality warnings (non-blocking)${NC}"
}

# Build the project
echo -e "${BLUE}🔨 Building project...${NC}"
./gradlew build --no-daemon

# Generate documentation
echo -e "${BLUE}📚 Generating documentation...${NC}"
./gradlew dokkaHtml --no-daemon || {
    echo -e "${YELLOW}⚠️  Documentation generation skipped${NC}"
}

# Get version from build.gradle.kts
VERSION=$(grep -oP 'version = "\K[^"]+' build.gradle.kts)

# Success message
echo ""
echo -e "${GREEN}✅ Build completed successfully!${NC}"
echo ""
echo "Package: com.hivellm:umicp-kotlin"
echo "Version: $VERSION"
echo ""
echo "Generated artifacts:"
ls -lh build/libs/*.jar 2>/dev/null || echo "No JARs found"
echo ""
echo "Test report: build/reports/tests/test/index.html"
echo "Coverage report: build/reports/jacoco/test/html/index.html"
echo "Documentation: build/docs/index.html"
echo ""
echo "Next steps:"
echo "  1. Test locally: ./gradlew publishToMavenLocal"
echo "  2. Publish to Maven Central: ./publish.sh"
echo ""

