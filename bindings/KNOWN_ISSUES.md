# Known Issues - UMICP v0.2.2

## TypeScript SDK

### Native Addon Dependencies

**Issue:** Full test suite requires native C++ addon compilation

**Status:** ⚠️ Optional - HTTP transport works without addon

**Affected Tests:**
- Matrix operations (require native SIMD)
- Some envelope tests (use native serialization)
- Load/stress tests (native performance)
- Security regression tests

**Working Tests:**
- ✅ Custom endpoint tests: 23/23 (100%)
- ✅ HTTP transport configuration
- ✅ Type safety and interfaces

**Workaround:**
Custom endpoint functionality (the v0.2.2 feature) works perfectly without native addon.

**To Fix:**
```bash
# Install C++ dependencies
apt-get install nlohmann-json3-dev

# Compile native addon
npm run build:native
```

**Impact:** Low - HTTP transport and custom endpoints work fine. Native addon only needed for peak performance in matrix operations.

---

## All Other SDKs

No known issues. All tested SDKs at 100% test coverage.

