# UMICP C++ Release Guide

This guide explains how to create and publish a new release of UMICP C++.

## Prerequisites

- Write access to the repository
- Git configured with your credentials
- All changes committed and pushed to main branch
- All tests passing

## Release Process

### 1. Prepare the Release

```bash
# Ensure you're on main branch with latest changes
git checkout main
git pull origin main

# Update version numbers in files
# - cpp/CMakeLists.txt (project VERSION)
# - cpp/CPack.cmake (CPACK_PACKAGE_VERSION_*)
# - cpp/conanfile.py (version)
# - vcpkg/vcpkg.json (version)
# - vcpkg/portfile.cmake (REF)

# Update CHANGELOG.md
# Add release notes for this version

# Commit version changes
git add .
git commit -m "Bump version to 0.1.1"
git push origin main
```

### 2. Create and Push Tag

```bash
# Create an annotated tag
git tag -a v0.1.1 -m "Release version 0.1.1

Features:
- Core protocol implementation
- WebSocket transport
- HTTP/HTTP2 support
- SIMD matrix operations
- Cross-platform support

See CHANGELOG.md for full details."

# Push the tag
git push origin v0.1.1
```

### 3. Automated Build Process

Once the tag is pushed, GitHub Actions will automatically:

1. **Build on Multiple Platforms**:
   - Linux (Ubuntu, x64) → `.tar.gz`, `.deb`
   - macOS (x64) → `.tar.gz`, `.zip`
   - Windows (x64) → `.zip`, `.exe`

2. **Run Tests**:
   - All unit tests
   - All integration tests
   - Platform-specific tests

3. **Create Packages**:
   - Binary distributions
   - Debug symbols
   - Documentation

4. **Create GitHub Release**:
   - Automatic release creation
   - Upload all artifacts
   - Generate release notes

### 4. Monitor the Build

```bash
# Check GitHub Actions
# Go to: https://github.com/hivellm/umicp/actions

# Wait for all jobs to complete (usually 10-20 minutes)
# - build-linux ✓
# - build-macos ✓
# - build-windows ✓
# - create-release ✓
```

### 5. Verify the Release

```bash
# Check the release page
# https://github.com/hivellm/umicp/releases/tag/v0.1.1

# Verify all development packages are present:
# ✓ umicp-dev-linux-x64.tar.gz   (shared libs + headers)
# ✓ umicp-dev-macos-x64.tar.gz   (dynamic libs + headers)
# ✓ umicp-dev-windows-x64.zip    (DLLs + headers + import libs)
```

### 6. Test the Release Artifacts

```bash
# Download and test Linux library package
wget https://github.com/hivellm/umicp/releases/download/v0.1.1/umicp-dev-linux-x64.tar.gz
tar -xzf umicp-dev-linux-x64.tar.gz

# Verify contents
ls -R umicp-dev/
# Should show:
# - include/umicp/*.hpp
# - lib/libumicp.so
# - lib/libumicp.a
# - lib/cmake/UMICP/*.cmake

# Test by compiling a simple program
cat > test.cpp << 'EOF'
#include <umicp/envelope.hpp>
#include <iostream>
int main() {
    umicp::Envelope env;
    env.setFrom("test");
    std::cout << "UMICP library works!" << std::endl;
    return 0;
}
EOF

g++ -std=c++17 -I./umicp-dev/include test.cpp -L./umicp-dev/lib -lumicp -o test
./test
```

### 7. Publish to Package Managers

#### Conan Center

```bash
# Fork https://github.com/conan-io/conan-center-index
# Follow steps in cpp/CONAN_PUBLISH.md

# Create PR with recipe
# Wait for CI to pass
# Wait for review and merge
```

#### vcpkg

```bash
# Fork https://github.com/Microsoft/vcpkg
# Follow steps in cpp/VCPKG_PUBLISH.md

# Create PR with port
# Wait for CI to pass
# Wait for review and merge
```

### 8. Announce the Release

```bash
# Update documentation site
# Post on social media / forums
# Send notification to users
# Update package manager listings
```

## Version Numbering

UMICP follows Semantic Versioning (SemVer):

- **MAJOR** (v**1**.0.0): Breaking changes
- **MINOR** (v0.**1**.0): New features, backwards compatible
- **PATCH** (v0.0.**1**): Bug fixes, backwards compatible

Examples:
- `v0.1.0` → `v0.1.1` - Bug fix release
- `v0.1.1` → `v0.2.0` - New features
- `v0.9.0` → `v1.0.0` - Stable release with potential breaking changes

## Release Checklist

Before creating a release:

- [ ] All tests passing locally
- [ ] CHANGELOG.md updated
- [ ] Version numbers updated in all files
- [ ] Documentation updated
- [ ] Breaking changes documented (if any)
- [ ] Migration guide provided (if breaking changes)
- [ ] Examples tested
- [ ] README.md updated
- [ ] Security vulnerabilities checked
- [ ] Dependencies updated (if needed)

## Hotfix Release Process

For critical bug fixes:

```bash
# Create hotfix branch from tag
git checkout -b hotfix/v0.1.2 v0.1.1

# Fix the bug
# ... make changes ...

# Test thoroughly
make test

# Commit fix
git commit -m "Fix critical bug in WebSocket transport"

# Update version to 0.1.2
# Commit version bump

# Create tag
git tag -a v0.1.2 -m "Hotfix: Critical WebSocket bug"

# Push tag
git push origin v0.1.2

# Merge back to main
git checkout main
git merge hotfix/v0.1.2
git push origin main
```

## Rolling Back a Release

If a release has critical issues:

```bash
# Delete the tag locally
git tag -d v0.1.1

# Delete the tag remotely
git push origin :refs/tags/v0.1.1

# Delete the GitHub release
# Go to: https://github.com/hivellm/umicp/releases
# Click on the release → Delete

# Note: This should be rare and avoided if possible
# Better to create a hotfix release
```

## Troubleshooting

### Build Fails on CI

1. Check GitHub Actions logs
2. Reproduce locally:
   ```bash
   cd cpp
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   cd build && ctest
   ```
3. Fix issues and push
4. Delete and recreate tag

### Release Not Created

1. Check workflow permissions
2. Verify tag format (`v*.*.*`)
3. Check GITHUB_TOKEN permissions
4. Re-run failed job

### Artifacts Missing

1. Check CPack configuration
2. Verify build succeeded
3. Check artifact upload step
4. Re-run workflow

## Post-Release Tasks

After a successful release:

1. **Update main branch**:
   ```bash
   # Bump to next dev version
   # e.g., 0.1.1 → 0.1.2-dev
   ```

2. **Close related issues**:
   - Tag with release version
   - Add to milestone

3. **Update documentation**:
   - API docs
   - Tutorial examples
   - Migration guides

4. **Monitor feedback**:
   - Check GitHub issues
   - Monitor downloads
   - Review user feedback

## Release Schedule

UMICP follows this release schedule:

- **Patch releases**: As needed (bug fixes)
- **Minor releases**: Every 2-3 months (new features)
- **Major releases**: Once per year (breaking changes)

## Support

- GitHub Issues: https://github.com/hivellm/umicp/issues
- Discussions: https://github.com/hivellm/umicp/discussions
- Documentation: https://github.com/hivellm/umicp/tree/main/docs

---

**Last Updated**: 2025-10-10  
**Current Version**: 0.1.1

