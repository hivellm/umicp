# Contributing to UMICP PHP Bindings

Thank you for your interest in contributing to UMICP PHP bindings!

## Development Setup

```bash
# Clone repository
git clone https://github.com/hivellm/umicp.git
cd umicp/bindings/php

# Install dependencies
composer install

# Build C++ core
./build-cpp.sh

# Run tests
./vendor/bin/phpunit
```

## Coding Standards

### PHP Version
- Minimum: PHP 8.1
- Use modern features: enums, named parameters, attributes

### Code Style
- **PSR-12**: Follow PSR-12 coding standard
- **Strict Types**: All files must use `declare(strict_types=1)`
- **Type Hints**: All methods must have complete type hints
- **PHPDoc**: All public methods must have PHPDoc comments

### Example

```php
<?php

declare(strict_types=1);

namespace UMICP\Core;

/**
 * Example class following standards
 */
class ExampleClass
{
    /**
     * Example method with full type hints and PHPDoc
     *
     * @param string $param Parameter description
     * @return bool Return value description
     */
    public function exampleMethod(string $param): bool
    {
        // Implementation
        return true;
    }
}
```

## Testing

### Writing Tests

```php
<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\ExampleClass;

class ExampleClassTest extends TestCase
{
    public function testExample(): void
    {
        $instance = new ExampleClass();
        $result = $instance->exampleMethod('test');
        
        $this->assertTrue($result);
    }
}
```

### Running Tests

```bash
# All tests
./vendor/bin/phpunit

# Specific suite
./vendor/bin/phpunit --testsuite=Unit

# With coverage
./vendor/bin/phpunit --coverage-text

# Performance tests (excluded by default)
./vendor/bin/phpunit --group=performance
```

## Code Quality

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

## Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Make** your changes
4. **Add** tests for new functionality
5. **Run** `composer check` to ensure quality
6. **Commit** your changes (`git commit -m 'Add amazing feature'`)
7. **Push** to your branch (`git push origin feature/amazing-feature`)
8. **Open** a Pull Request

### PR Checklist

- [ ] Code follows PSR-12
- [ ] All files have `declare(strict_types=1)`
- [ ] All methods have type hints
- [ ] Public methods have PHPDoc
- [ ] Tests added for new features
- [ ] All tests pass (`composer test`)
- [ ] Code style check passes (`composer lint`)
- [ ] Static analysis passes (`composer analyse`)
- [ ] Documentation updated if needed

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

