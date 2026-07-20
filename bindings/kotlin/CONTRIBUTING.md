# Contributing to UMICP Kotlin SDK

Thank you for your interest in contributing! This document provides guidelines for contributing to the UMICP Kotlin SDK.

## Development Setup

### Prerequisites
- **JDK**: 17 or higher
- **Kotlin**: 1.9.21 or higher
- **Gradle**: 8.0 or higher
- **Git**: Latest version

### Getting Started

1. **Fork and Clone**
   ```bash
   git clone https://github.com/YOUR_USERNAME/umicp.git
   cd umicp/bindings/kotlin
   ```

2. **Build**
   ```bash
   ./gradlew build
   ```

3. **Run Tests**
   ```bash
   ./gradlew test
   ```

4. **Run Examples**
   ```bash
   ./gradlew run --args="com.hivellm.umicp.examples.BasicExample"
   ```

## Code Style

### Kotlin Conventions
Follow the [official Kotlin coding conventions](https://kotlinlang.org/docs/coding-conventions.html):

- Use 4 spaces for indentation
- Use camelCase for function and property names
- Use PascalCase for class names
- Use UPPER_SNAKE_CASE for constants
- Place opening braces on the same line
- Use trailing commas in multi-line constructs

### Example
```kotlin
data class ServiceInfo(
    val id: String,
    val name: String,
    val endpoint: String,
    val capabilities: Set<String> = emptySet(),
    val metadata: Map<String, String> = emptyMap(),
)

fun processService(service: ServiceInfo) {
    when {
        service.capabilities.isEmpty() -> handleEmpty()
        service.capabilities.size > 10 -> handleMany()
        else -> handleNormal()
    }
}
```

### Documentation
- Add KDoc comments for all public APIs
- Include examples in documentation
- Use `@param`, `@return`, `@throws` tags
- Keep documentation concise and clear

### Example
```kotlin
/**
 * Compute cosine similarity between two vectors
 * 
 * @param a First vector
 * @param b Second vector
 * @return Cosine similarity in range [-1, 1]
 * @throws MatrixException if vectors have different dimensions
 */
fun cosineSimilarity(a: FloatArray, b: FloatArray): MatrixResult<Float>
```

## Testing

### Test Structure
- Place tests in `src/test/kotlin/`
- Mirror the source package structure
- Use descriptive test names with backticks

### Example
```kotlin
class EnvelopeTest {
    @Test
    fun `should create envelope with required fields`() {
        val envelope = Envelope(
            from = "client",
            to = "server",
            operation = OperationType.DATA
        )
        
        envelope.from shouldBe "client"
    }
    
    @Test
    fun `should fail validation with blank from`() {
        val envelope = Envelope(from = "", to = "server", operation = OperationType.DATA)
        
        shouldThrow<ValidationException> {
            envelope.validateOrThrow()
        }
    }
}
```

### Test Coverage
- Aim for >90% code coverage
- Test happy paths and error cases
- Test edge cases and boundary conditions
- Use Kotest assertions for readability

### Running Tests
```bash
# All tests
./gradlew test

# Specific test class
./gradlew test --tests "com.hivellm.umicp.core.EnvelopeTest"

# With coverage
./gradlew test jacocoTestReport
```

## Pull Request Process

1. **Create a Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make Changes**
   - Write code following style guidelines
   - Add tests for new functionality
   - Update documentation

3. **Commit**
   ```bash
   git add .
   git commit -m "feat: add feature description"
   ```
   
   Use conventional commits:
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation changes
   - `test:` - Test changes
   - `refactor:` - Code refactoring
   - `perf:` - Performance improvements

4. **Push and Create PR**
   ```bash
   git push origin feature/your-feature-name
   ```
   
   Then create a Pull Request on GitHub

5. **PR Requirements**
   - All tests pass
   - Code coverage maintained/improved
   - Documentation updated
   - No linting errors
   - Reviewed and approved

## Areas for Contribution

### High Priority
- HTTP/2 transport implementation
- Compression support (GZIP/LZ4)
- Performance optimizations
- Additional tests

### Medium Priority
- Kotlin Multiplatform support
- Flow integration
- Metrics and monitoring
- Benchmarking suite

### Documentation
- More examples
- Tutorial content
- API improvements
- Video demonstrations

### Nice to Have
- IDE plugins
- Code generation tools
- Migration tools
- Performance profiling

## Questions or Need Help?

- Open an [issue](https://github.com/hivellm/umicp/issues)
- Start a [discussion](https://github.com/hivellm/umicp/discussions)
- Check existing documentation

## Code of Conduct

- Be respectful and inclusive
- Welcome newcomers
- Focus on constructive feedback
- Help others learn and grow

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

