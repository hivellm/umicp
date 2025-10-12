#!/bin/bash
# Publish script for UMICP Kotlin SDK to Maven Central
# Usage: ./publish.sh

set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${BLUE}📤 Publishing UMICP Kotlin SDK to Maven Central...${NC}"

# Check if Java is installed
if ! command -v java &> /dev/null; then
    echo -e "${RED}❌ Java not found.${NC}"
    exit 1
fi

# Get version from build.gradle.kts
VERSION=$(grep -oP 'version = "\K[^"]+' build.gradle.kts)

echo "Package: com.hivellm:umicp-kotlin"
echo "Version: $VERSION"
echo ""

# Check if artifacts exist
if [ ! -d "build/libs" ] || [ -z "$(ls -A build/libs/*.jar 2>/dev/null)" ]; then
    echo -e "${YELLOW}⚠️  No build artifacts found. Running build...${NC}"
    ./build.sh
fi

# Check for signing credentials
if [ -z "$ORG_GRADLE_PROJECT_signingKey" ] && [ -z "$ORG_GRADLE_PROJECT_signingKeyId" ]; then
    echo -e "${YELLOW}⚠️  No signing credentials found!${NC}"
    echo ""
    echo "For Maven Central, you need to configure signing."
    echo ""
    echo "Set environment variables:"
    echo "  export ORG_GRADLE_PROJECT_signingKeyId=YOUR_KEY_ID"
    echo "  export ORG_GRADLE_PROJECT_signingKey=YOUR_GPG_KEY"
    echo "  export ORG_GRADLE_PROJECT_signingPassword=YOUR_PASSWORD"
    echo ""
    echo "  export ORG_GRADLE_PROJECT_ossrhUsername=YOUR_SONATYPE_USERNAME"
    echo "  export ORG_GRADLE_PROJECT_ossrhPassword=YOUR_SONATYPE_PASSWORD"
    echo ""
    echo "Or add to ~/.gradle/gradle.properties:"
    echo "  signingKeyId=YOUR_KEY_ID"
    echo "  signingKey=YOUR_GPG_KEY"
    echo "  signingPassword=YOUR_PASSWORD"
    echo "  ossrhUsername=YOUR_SONATYPE_USERNAME"
    echo "  ossrhPassword=YOUR_SONATYPE_PASSWORD"
    echo ""
    read -p "Continue with publish to local Maven? (yes/no): " local_only
    if [ "$local_only" != "yes" ]; then
        echo "Publish cancelled."
        exit 0
    fi

    # Publish to local Maven
    echo -e "${BLUE}📦 Publishing to local Maven repository...${NC}"
    ./gradlew publishToMavenLocal --no-daemon

    echo ""
    echo -e "${GREEN}✅ Published to local Maven!${NC}"
    echo ""
    echo "Package available at: ~/.m2/repository/com/hivellm/umicp-kotlin/$VERSION/"
    echo ""
    echo "Test installation:"
    echo "  Add to build.gradle.kts:"
    echo "    repositories { mavenLocal() }"
    echo "    dependencies { implementation(\"com.hivellm:umicp-kotlin:$VERSION\") }"
    echo ""
    exit 0
fi

# Confirm publish (unless in CI)
if [ -z "$CI" ]; then
    echo -e "${YELLOW}⚠️  You are about to publish to Maven Central!${NC}"
    echo ""
    read -p "Continue with publish to Maven Central? (yes/no): " confirm
    if [ "$confirm" != "yes" ]; then
        echo "Publish cancelled."
        exit 0
    fi
fi

# Publish to Maven Central
echo -e "${BLUE}📤 Publishing to Maven Central...${NC}"
./gradlew publish --no-daemon

# Success message
echo ""
echo -e "${GREEN}✅ Package published successfully!${NC}"
echo ""
echo "View on Maven Central:"
echo "  https://central.sonatype.com/artifact/com.hivellm/umicp-kotlin/$VERSION"
echo ""
echo "Installation:"
echo "  implementation(\"com.hivellm:umicp-kotlin:$VERSION\")"
echo ""
echo "Note: It may take a few hours for the package to be available on Maven Central."
echo ""

