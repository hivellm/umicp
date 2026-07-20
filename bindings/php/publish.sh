#!/bin/bash
# Publish script for UMICP PHP SDK to Packagist
# Packagist automatically pulls from Git tags, so this script creates and pushes a tag

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

echo -e "${BLUE}📤 Publishing UMICP PHP SDK to Packagist...${NC}"

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo -e "${RED}❌ Git not found.${NC}"
    exit 1
fi

# Get package version from argument or prompt
VERSION="$1"

if [ -z "$VERSION" ]; then
    echo -e "${YELLOW}No version provided as argument${NC}"
    read -p "Enter version to release (e.g., 0.1.3): " VERSION
fi

if [ -z "$VERSION" ]; then
    echo -e "${RED}❌ Version is required${NC}"
    exit 1
fi

# Remove 'v' prefix if present
VERSION="${VERSION#v}"

echo "Package: hivellm/umicp"
echo "Version: $VERSION"
echo ""

# Check if there are uncommitted changes
if ! git diff-index --quiet HEAD --; then
    echo -e "${YELLOW}⚠️  You have uncommitted changes!${NC}"
    echo ""
    git status --short
    echo ""
    read -p "Commit changes before tagging? (yes/no): " commit_confirm
    if [ "$commit_confirm" == "yes" ]; then
        echo ""
        read -p "Commit message: " commit_msg
        git add .
        git commit -m "$commit_msg"
        echo -e "${GREEN}✅ Changes committed${NC}"
    else
        echo -e "${YELLOW}⚠️  Proceeding with uncommitted changes...${NC}"
    fi
fi

# Check if tag already exists
if git rev-parse "v$VERSION" >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  Tag v$VERSION already exists!${NC}"
    read -p "Delete and recreate tag? (yes/no): " tag_confirm
    if [ "$tag_confirm" == "yes" ]; then
        git tag -d "v$VERSION"
        git push origin ":refs/tags/v$VERSION" 2>/dev/null || true
        echo -e "${GREEN}✅ Old tag deleted${NC}"
    else
        echo -e "${RED}❌ Cannot proceed with existing tag${NC}"
        exit 1
    fi
fi

# Create annotated tag
echo -e "${BLUE}🏷️  Creating tag v$VERSION...${NC}"
git tag -a "v$VERSION" -m "Release v$VERSION

- UMICP PHP SDK production release
- Full BIP-05 protocol compliance
- 115+ tests with 95% coverage
- WebSocket transport (client/server)
- Multiplexed peer architecture
- Service Discovery & Connection Pooling
- Compression support (GZIP/DEFLATE)
- Production ready

See CHANGELOG.md for details."

echo -e "${GREEN}✅ Tag created${NC}"

# Confirm push (unless in CI)
if [ -z "$CI" ]; then
    echo ""
    echo -e "${YELLOW}⚠️  Ready to push to remote repository!${NC}"
    echo "This will trigger Packagist auto-update (if configured)"
    echo ""
    read -p "Push tag v$VERSION to origin? (yes/no): " push_confirm
    if [ "$push_confirm" != "yes" ]; then
        echo "Push cancelled. Tag created locally only."
        echo "To push later: git push origin v$VERSION"
        exit 0
    fi
fi

# Push tag to remote
echo -e "${BLUE}📤 Pushing tag to origin...${NC}"
git push origin "v$VERSION"

echo ""
echo -e "${GREEN}✅ Tag pushed successfully!${NC}"
echo ""
echo "📦 Packagist Publication:"
echo ""
echo "If your package is already on Packagist with GitHub webhook:"
echo "  → Packagist will auto-update within minutes"
echo "  → View at: https://packagist.org/packages/hivellm/umicp"
echo ""
echo "If this is your first release:"
echo "  1. Go to: https://packagist.org/packages/submit"
echo "  2. Submit: https://github.com/hivellm/umicp"
echo "  3. Configure GitHub Service Hook for auto-updates"
echo ""
echo "Installation:"
echo "  composer require hivellm/umicp:$VERSION"
echo ""

