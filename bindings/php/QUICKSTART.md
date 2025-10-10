# UMICP PHP Bindings - Quick Start Guide

> Get up and running with UMICP PHP bindings in 5 minutes!

## Prerequisites

```bash
# Check PHP version (8.1+ required)
php -v

# Check FFI extension
php -m | grep FFI

# If FFI missing, install it
sudo apt-get install php-ffi  # Ubuntu/Debian
```

## Installation

### Option 1: Automated Setup (Recommended)

```bash
cd umicp/bindings/php
chmod +x setup.sh
./setup.sh
```

This script will:
1. Check PHP version and FFI
2. Install Composer dependencies
3. Build C++ core library
4. Copy configuration
5. Validate structure

### Option 2: Manual Setup

```bash
# 1. Install dependencies
composer install

# 2. Build C++ core
chmod +x build-cpp.sh
./build-cpp.sh

# 3. Configure
cp config/umicp.example.php config/umicp.php
# Edit config/umicp.php with correct library path

# 4. Validate
php validate-structure.php
```

## Quick Examples

### Example 1: Basic Envelope

```php
<?php

require 'vendor/autoload.php';

use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

// Create envelope
$envelope = new Envelope(
    from: 'client-001',
    to: 'server-001',
    operation: OperationType::DATA,
    capabilities: ['message' => 'Hello UMICP!']
);

// Serialize
$json = $envelope->serialize();
echo $json . "\n";

// Deserialize
$received = Envelope::deserialize($json);
echo "From: {$received->getFrom()}\n";

// Validate
if ($envelope->validate()) {
    echo "✓ Valid envelope\n";
}
```

**Run**:
```bash
php examples/01_basic_envelope.php
```

### Example 2: Matrix Operations

```php
<?php

require 'vendor/autoload.php';

use UMICP\Core\Matrix;

$matrix = new Matrix();

// Dot product
$result = $matrix->dotProduct([1, 2, 3], [4, 5, 6]);
echo "Dot product: $result\n"; // 32.0

// Cosine similarity
$similarity = $matrix->cosineSimilarity([1, 2, 3], [4, 5, 6]);
echo "Similarity: $similarity\n"; // ~0.974

// Vector operations
$sum = $matrix->vectorAdd([1, 2, 3], [4, 5, 6]);
print_r($sum); // [5, 7, 9]
```

**Run**:
```bash
php examples/02_matrix_operations.php
```

### Example 3: Complete Demo

```bash
php examples/03_complete_demo.php
```

This demonstrates:
- FFI initialization
- Envelope CRUD operations
- Matrix calculations
- Frame operations
- Configuration system
- All enums
- Performance metrics

## Common Issues

### Issue 1: FFI Not Enabled

```
Error: FFI extension is not loaded
```

**Solution**:
```ini
; php.ini
extension=ffi
ffi.enable=1
```

### Issue 2: Library Not Found

```
Error: C++ library not found
```

**Solution**:
```bash
# Build C++ core
./build-cpp.sh

# Update config
vim config/umicp.php
# Set correct lib_path
```

### Issue 3: Composer Not Found

```bash
# Install Composer
curl -sS https://getcomposer.org/installer | php
mv composer.phar /usr/local/bin/composer
```

## Testing

```bash
# Run all tests
./vendor/bin/phpunit

# Run specific test
./vendor/bin/phpunit tests/Unit/Core/EnvelopeTest.php

# With coverage
./vendor/bin/phpunit --coverage-text
```

## Development

```bash
# Check code style
composer lint

# Fix code style
composer lint:fix

# Static analysis
composer analyse

# Run all checks
composer check
```

## Next Steps

1. **Explore Examples**: Run all examples in `examples/`
2. **Read API Docs**: See `docs/API_SPECIFICATION.md`
3. **Build Something**: Try the API in your own code
4. **Contribute**: See contributing guidelines

## Support

- **Documentation**: [docs/INDEX.md](docs/INDEX.md)
- **Issues**: https://github.com/hivellm/umicp/issues
- **Email**: dev@hivellm.org

---

**Ready to build!** 🚀

