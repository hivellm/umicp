# Changelog

All notable changes to the UMICP Python SDK will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.0] - 2025-10-24

### Changed - Package Renamed for PEP 625 Compliance

**BREAKING CHANGE**: Package renamed from `umicp-python` to `umicp-sdk`

- **Package name**: `umicp-python` → `umicp-sdk` (PyPI display name)
- **Module name**: `umicp` → `umicp_sdk` (import name, PEP 625 compliant)
- **Protocol**: UMICP (Universal Matrix Inter-Communication Protocol)

### Migration Guide

**Before (v0.2.2)**:
```python
pip install umicp-python
from umicp import Envelope, OperationType
```

**After (v0.3.0)**:
```python
pip install umicp-sdk
from umicp_sdk import Envelope, OperationType
```

### What Changed

1. **Package Installation**:
   - Old: `pip install umicp-python`
   - New: `pip install umicp-sdk`

2. **Import Statements**:
   - Old: `from umicp import ...`
   - New: `from umicp_sdk import ...`

3. **Internal Structure**:
   - All internal imports updated to use `umicp_sdk`
   - Module structure remains identical
   - All functionality preserved

### Why This Change

- **PEP 625 Compliance**: New package name uses underscores (`umicp_sdk`) as required by PyPI
- **Clearer Naming**: SDK suffix makes package purpose clearer
- **Better Organization**: Separates SDK from core protocol

### Compatibility

- ✅ All APIs remain identical (only import paths changed)
- ✅ All features from v0.2.2 included
- ✅ No functionality removed
- ❌ Old package name `umicp-python` deprecated (use `umicp-sdk` going forward)

---

## [0.2.2] - 2025-10-23

### Features

- ✅ Complete envelope system with JSON serialization
- ✅ NumPy-powered matrix operations
- ✅ Async WebSocket client and server
- ✅ HTTP/2 client and server support
- ✅ Multiplexed peer architecture
- ✅ Event system with async emitter
- ✅ Service discovery
- ✅ Connection pooling
- ✅ GZIP/DEFLATE compression
- ✅ Tool discovery protocol
- ✅ Comprehensive test suite (100+ tests)
- ✅ Full type hints (PEP 561 compliant)

### Status

- **Python Version**: 3.9+
- **Coverage**: 95%+
- **Dependencies**: Modern async stack
- **Production Ready**: Yes

---

## Historical Versions

Previous versions (0.1.0 - 0.2.1) were published under `umicp-python` package name.
See git history for detailed changelog of these versions.

