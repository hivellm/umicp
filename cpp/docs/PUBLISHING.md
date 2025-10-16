# UMICP Publishing Guide

**Version**: 0.2.0  
**Last Updated**: 2025-10-16

This document consolidates all publishing instructions for UMICP across different package managers.

---

## 📦 Package Managers

### 1. Conan (C++ Package Manager)

**Status**: Ready for v0.2.0 submission

#### Requirements
- Conan 2.0+
- GCC 11+ or Clang 14+
- CMake 3.16+

#### Build and Test
```bash
cd umicp/cpp
conan create . --build=missing
conan test test_package umicp/0.2.0@
```

#### Publish to Conan Center
```bash
# 1. Fork conan-center-index
git clone https://github.com/conan-io/conan-center-index.git
cd conan-center-index

# 2. Create recipe
mkdir recipes/umicp/0.2.0
cp /path/to/conanfile.py recipes/umicp/0.2.0/

# 3. Test locally
conan create recipes/umicp/all --version=0.2.0

# 4. Submit PR
git checkout -b umicp-0.2.0
git add recipes/umicp
git commit -m "Add umicp/0.2.0"
git push origin umicp-0.2.0
# Create PR to conan-center-index
```

---

### 2. vcpkg (Microsoft Package Manager)

**Status**: Ready for v0.2.0 submission

#### Requirements
- vcpkg installed
- Git

#### Local Test
```bash
cd vcpkg
./vcpkg install umicp --overlay-ports=/path/to/umicp/vcpkg
```

#### Publish to vcpkg Registry
```bash
# 1. Fork vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# 2. Create port
mkdir ports/umicp
cat > ports/umicp/portfile.cmake << 'EOF'
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO hivellm/umicp
    REF v0.2.0
    SHA512 <calculate>
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/cpp"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/umicp" RENAME copyright)
EOF

# 3. Test
./vcpkg install umicp

# 4. Submit PR
git checkout -b add-umicp-0.2.0
git add ports/umicp
git commit -m "[umicp] Add new port 0.2.0"
git push origin add-umicp-0.2.0
```

---

### 3. GitHub Releases

#### Create Release Tag
```bash
cd umicp
git checkout feat/tool-discovery-native-types
git tag -a v0.2.0 -m "UMICP v0.2.0 - Native Types + Tool Discovery

- Native JSON type support in capabilities
- Tool discovery interfaces (MCP-compatible)
- 84 tests passing (100% coverage)
- C++ Core + Rust binding aligned

BREAKING CHANGES:
- Capabilities: HashMap<String, String> → HashMap<String, json::Value>
- See CHANGELOG.md for migration guide
"

git push origin v0.2.0
```

#### Create GitHub Release
1. Go to https://github.com/hivellm/umicp/releases/new
2. Select tag: `v0.2.0`
3. Title: "UMICP v0.2.0 - Native Types & Tool Discovery"
4. Description: Copy from CHANGELOG.md
5. Attach artifacts:
   - Source code (auto)
   - `libumicp-0.2.0.tar.gz` (optional)
6. Mark as "Pre-release" if needed
7. Publish

---

### 4. Cargo (Rust Crates)

**Package**: `umicp-core`  
**Status**: Ready for v0.2.0 publication

#### Publish to crates.io
```bash
cd umicp/bindings/rust

# Verify
cargo package --list
cargo package

# Publish
cargo publish --dry-run
cargo publish
```

**Registry**: https://crates.io/crates/umicp-core

---

### 5. npm (TypeScript/JavaScript)

**Package**: `@umicp/core`  
**Status**: Pending v0.2.0 update

```bash
cd umicp/bindings/typescript

# Update version in package.json
npm version 0.2.0

# Publish
npm publish --access public
```

---

### 6. PyPI (Python)

**Package**: `umicp-python`  
**Status**: Pending v0.2.0 update

```bash
cd umicp/bindings/python

# Build
python setup.py sdist bdist_wheel

# Publish
twine upload dist/*
```

---

### 7. Maven Central (Java)

**Package**: `com.hivellm:umicp-core`  
**Status**: Pending v0.2.0 update

```bash
cd umicp/bindings/java

# Build and sign
mvn clean deploy -P release

# Or via Sonatype
mvn clean deploy
```

---

### 8. NuGet (C#)

**Package**: `Umicp.Core`  
**Status**: Pending v0.2.0 update

```bash
cd umicp/bindings/csharp

# Build
dotnet pack -c Release

# Publish
dotnet nuget push bin/Release/Umicp.Core.0.2.0.nupkg --source https://api.nuget.org/v3/index.json
```

---

## 📋 Pre-Release Checklist

### Before Publishing

- [x] All tests passing (84/84 ✅)
- [x] CHANGELOG.md updated
- [x] Version bumped in all files
- [x] Documentation updated
- [x] Migration guide complete
- [ ] Security audit completed
- [ ] Performance benchmarks run
- [ ] Breaking changes clearly documented
- [ ] Examples updated
- [ ] CI/CD passing

### Package-Specific

#### C++ (Conan/vcpkg)
- [x] CMakeLists.txt version updated
- [x] Headers installed correctly
- [x] Libraries link properly
- [ ] conanfile.py created
- [ ] vcpkg portfile.cmake created

#### Rust (Cargo)
- [x] Cargo.toml version 0.2.0
- [x] All tests passing
- [x] CHANGELOG.md updated
- [ ] cargo publish dry-run successful

#### Other Bindings
- [ ] TypeScript: package.json updated
- [ ] Python: setup.py updated
- [ ] Java: pom.xml updated
- [ ] C#: .csproj updated

---

## 🔄 Release Process

### 1. Prepare
1. Merge `feat/tool-discovery-native-types` to `main`
2. Verify all tests on `main`
3. Update all CHANGELOGs
4. Create git tag `v0.2.0`

### 2. Build
1. Build C++ libraries (Release mode)
2. Build all language bindings
3. Run full test suite
4. Generate documentation

### 3. Publish (in order)
1. **GitHub Release** - Create release with tag
2. **Cargo** - Publish Rust binding first
3. **Conan** - Submit C++ core to Conan Center
4. **vcpkg** - Submit C++ core to vcpkg
5. **npm** - Publish TypeScript (after update)
6. **PyPI** - Publish Python (after update)
7. **Maven** - Publish Java (after update)
8. **NuGet** - Publish C# (after update)

### 4. Announce
1. Update README.md badges
2. Announce on GitHub Discussions
3. Update documentation site
4. Notify users of breaking changes

---

## 📝 Notes

### Breaking Changes in v0.2.0

**ALL USERS MUST UPDATE**:
- Capabilities API changed
- See CHANGELOG.md for migration
- Old binaries not compatible with new headers

### Backward Compatibility

- **Wire Protocol**: Compatible (JSON unchanged)
- **API**: NOT compatible (breaking changes)
- **Migration**: Required for all users

---

**Maintained by**: HiveLLM Team  
**Support**: GitHub Issues

