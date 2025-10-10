# GitHub Actions Workflows

This directory contains GitHub Actions workflows for automated builds, tests, and releases.

## Workflows

### 🚀 Release Build (`release.yml`)

Automatically builds and publishes releases when you push a version tag.

#### Trigger

```bash
# Create and push a tag
git tag v0.1.1
git push origin v0.1.1
```

#### What it does

1. **Multi-platform builds**:
   - Linux (Ubuntu) - x64
   - macOS (latest) - x64
   - Windows (latest) - x64

2. **Compilation**:
   - Builds in Release mode
   - Runs all tests
   - Builds examples

3. **Packaging**:
   - **Linux**: `.tar.gz`, `.deb`
   - **macOS**: `.tar.gz`, `.zip`
   - **Windows**: `.zip`, `.exe` (NSIS installer)

4. **GitHub Release**:
   - Creates a GitHub release automatically
   - Uploads all platform artifacts
   - Generates release notes

5. **Optional Conan Publishing**:
   - Creates Conan package
   - Ready to publish to Conan Center (commented out)

#### Artifacts Generated

For version `v0.1.1`, the following artifacts are created:

**Linux**:
- `umicp-0.1.1-Linux.tar.gz` - Binary tarball
- `umicp-0.1.1-Linux.deb` - Debian package

**macOS**:
- `umicp-0.1.1-Darwin.tar.gz` - Binary tarball
- `umicp-0.1.1-Darwin.zip` - ZIP archive

**Windows**:
- `umicp-0.1.1-win64.zip` - ZIP archive
- `umicp-0.1.1-win64.exe` - NSIS installer

#### Usage Example

```bash
# 1. Make changes and commit
git add .
git commit -m "Release version 0.1.1"

# 2. Create and push tag
git tag v0.1.1
git push origin v0.1.1

# 3. GitHub Actions will:
#    - Build on all platforms
#    - Run tests
#    - Create packages
#    - Create GitHub Release
#    - Upload artifacts

# 4. View release at:
#    https://github.com/hivellm/umicp/releases/tag/v0.1.1
```

## Requirements

### For Release Workflow

The workflow uses:
- `actions/checkout@v4` - Checkout code
- `actions/upload-artifact@v4` - Upload build artifacts
- `actions/download-artifact@v4` - Download artifacts
- `softprops/action-gh-release@v1` - Create GitHub release
- `lukka/run-vcpkg@v11` - Setup vcpkg (Windows)

### Secrets (Optional)

For publishing to Conan Center, add these secrets in GitHub:
- `CONAN_LOGIN_USERNAME` - Conan username
- `CONAN_PASSWORD` - Conan password

Go to: Repository → Settings → Secrets and variables → Actions

## Platform-Specific Notes

### Linux
- Uses apt-get for dependencies
- Builds both DEB and tarball
- Tests run with ctest

### macOS
- Uses Homebrew for dependencies
- Handles OpenSSL path correctly
- Builds tarball and ZIP

### Windows
- Uses vcpkg for dependencies
- Builds with Visual Studio 2022
- Creates ZIP and NSIS installer
- Requires vcpkg to be cached

## Troubleshooting

### Build Fails

1. **Check dependencies**:
   - Ensure all required libraries are listed
   - Verify package names for each platform

2. **Check CMake configuration**:
   - Review CMakeLists.txt
   - Check feature flags

3. **View logs**:
   - Go to Actions tab in GitHub
   - Click on failed workflow
   - Expand failed step

### Release Not Created

1. **Check permissions**:
   - Workflow needs `contents: write` permission
   - Verify in Settings → Actions → General

2. **Check tag format**:
   - Must match `v*.*.*` pattern
   - Examples: `v0.1.1`, `v1.0.0`

3. **Check GITHUB_TOKEN**:
   - Should be automatically available
   - Check if repository has Actions enabled

### Artifacts Missing

1. **Check artifact upload step**:
   - Verify paths in workflow
   - Check if files were created in build

2. **Check retention**:
   - Artifacts kept for 90 days by default
   - Can be changed in workflow

## Customization

### Change Platforms

Edit `release.yml`:

```yaml
# Add ARM64 Linux
build-linux-arm64:
  runs-on: ubuntu-latest
  steps:
    # Use cross-compilation or ARM runner
```

### Change Package Formats

Edit `CPack.cmake`:

```cmake
# Add RPM for Linux
cpack -G "TGZ;DEB;RPM" -C $BUILD_TYPE
```

### Add More Tests

Edit workflow:

```yaml
- name: Run integration tests
  working-directory: cpp
  run: |
    cd build
    ctest -C $BUILD_TYPE -L integration
```

## CI/CD Best Practices

1. **Version Tags**: Always use semantic versioning (v0.1.1)
2. **Test Before Tag**: Run tests locally before pushing tag
3. **Release Notes**: Update CHANGELOG.md before release
4. **Artifacts**: Keep artifacts for at least 90 days
5. **Security**: Never commit secrets to workflow files

## Support

- GitHub Actions Docs: https://docs.github.com/actions
- CPack Documentation: https://cmake.org/cmake/help/latest/module/CPack.html
- Issue Tracker: https://github.com/hivellm/umicp/issues

---

**Last Updated**: 2025-10-10  
**Workflow Version**: 1.0.0

