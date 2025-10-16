# Contributing to UMICP Java Bindings

Thank you for your interest in contributing to UMICP Java Bindings! This document provides guidelines and instructions for contributing.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Project Structure](#project-structure)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Release Process](#release-process)

## Code of Conduct

This project adheres to the [Contributor Covenant Code of Conduct](../../CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally
3. **Create a feature branch** from `main`
4. **Make your changes** following our coding standards
5. **Add tests** for new functionality
6. **Ensure all tests pass**
7. **Submit a pull request**

## Development Setup

### Prerequisites

- **Java Development Kit (JDK)**: 17 or higher (Java 21+ recommended)
- **Maven**: 3.8+ or **Gradle**: 8+
- **Git**: Latest version
- **IDE**: IntelliJ IDEA, Eclipse, or VS Code with Java extensions
- **C++ Toolchain**: For building JNI components (GCC/Clang/MSVC)

### Initial Setup

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/umicp.git
cd umicp/bindings/java

# Build the project (Maven)
mvn clean install

# Or with Gradle
gradle build

# Run tests
mvn test
# Or
gradle test
```

### IDE Configuration

#### IntelliJ IDEA
1. Open the project: `File > Open` → Select `bindings/java` directory
2. Configure JDK: `File > Project Structure > SDKs`
3. Enable annotation processing
4. Install recommended plugins: Lombok (if used), SonarLint

#### Eclipse
1. Import as Maven/Gradle project
2. Configure JDK in project properties
3. Enable annotation processing

#### VS Code
1. Install Java Extension Pack
2. Open `bindings/java` folder
3. Configure Java runtime in settings

## Project Structure

```
bindings/java/
├── pom.xml                     # Maven configuration
├── build.gradle.kts            # Gradle configuration (alternative)
├── README.md                   # Main documentation
├── IMPLEMENTATION_ROADMAP.md   # Detailed roadmap
├── CONTRIBUTING.md             # This file
├── umicp-core/                 # Core module
│   ├── src/
│   │   ├── main/
│   │   │   ├── java/
│   │   │   │   └── com/hivellm/umicp/
│   │   │   │       ├── core/       # Envelope, Matrix, Frame
│   │   │   │       ├── types/      # Enums, exceptions
│   │   │   │       └── util/       # Utilities
│   │   │   └── resources/
│   │   └── test/
│   │       └── java/
│   └── pom.xml
├── umicp-transport/            # Transport module
│   ├── src/
│   │   ├── main/
│   │   │   └── java/
│   │   │       └── com/hivellm/umicp/
│   │   │           ├── transport/  # WebSocket
│   │   │           └── http/       # HTTP/2
│   │   └── test/
│   └── pom.xml
├── umicp-examples/             # Example applications
└── umicp-benchmark/            # Performance benchmarks
```

## Coding Standards

### Java Style Guide

We follow the **Google Java Style Guide** with some modifications:

#### Formatting
- **Indentation**: 4 spaces (not tabs)
- **Line length**: 120 characters
- **Braces**: K&R style (opening brace on same line)
- **Imports**: Group and order alphabetically

#### Naming Conventions
- **Classes**: PascalCase (e.g., `WebSocketClient`)
- **Methods**: camelCase (e.g., `connectToPeer`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_CONNECTIONS`)
- **Variables**: camelCase (e.g., `messageQueue`)
- **Packages**: lowercase (e.g., `com.hivellm.umicp.transport`)

#### JavaDoc
All public APIs must have JavaDoc:

```java
/**
 * Connects to a remote peer using WebSocket.
 *
 * @param url the WebSocket URL to connect to
 * @param metadata optional metadata to send during handshake
 * @return a CompletableFuture that completes with the peer ID
 * @throws ConnectionException if connection fails
 * @since 1.0.0
 */
public CompletableFuture<String> connectToPeer(String url, Map<String, String> metadata) {
    // Implementation
}
```

#### Error Handling
- Use checked exceptions for recoverable errors
- Use unchecked exceptions for programming errors
- Always include meaningful error messages
- Log errors with appropriate context

```java
try {
    envelope.validate();
} catch (ValidationException e) {
    logger.error("Envelope validation failed: {}", e.getMessage());
    throw new UMICPException("Invalid envelope", e);
}
```

#### Resource Management
Always use try-with-resources:

```java
try (Envelope envelope = UMICP.createEnvelope();
     Matrix matrix = UMICP.createMatrix()) {
    // Use resources
}
```

### Code Quality Tools

- **Checkstyle**: Enforce coding standards
- **SpotBugs**: Find bugs
- **PMD**: Code analysis
- **JaCoCo**: Code coverage (minimum 80%)
- **SonarQube**: Comprehensive analysis

Run quality checks:
```bash
mvn checkstyle:check
mvn spotbugs:check
mvn pmd:check
mvn jacoco:check
```

## Testing

### Test Structure

```
src/test/java/com/hivellm/umicp/
├── core/
│   ├── EnvelopeTest.java
│   └── MatrixTest.java
├── transport/
│   ├── WebSocketClientTest.java
│   └── WebSocketServerTest.java
└── integration/
    └── PeerToPeerIntegrationTest.java
```

### Test Categories

#### Unit Tests
Test individual classes in isolation:

```java
@Test
void testEnvelopeBuilder() {
    Envelope envelope = new Envelope()
        .setFrom("sender")
        .setTo("receiver")
        .setOperation(OperationType.DATA);
    
    assertThat(envelope.getFrom()).isEqualTo("sender");
    assertThat(envelope.getTo()).isEqualTo("receiver");
    assertThat(envelope.getOperation()).isEqualTo(OperationType.DATA);
}
```

#### Integration Tests
Test component interactions:

```java
@Test
void testClientServerCommunication() throws Exception {
    try (UMICPWebSocketServer server = new UMICPWebSocketServer(8080);
         UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080")) {
        
        server.start().get();
        client.connect().get();
        
        Envelope message = new Envelope()
            .setFrom("client")
            .setTo("server")
            .setOperation(OperationType.DATA);
        
        client.send(message).get();
        
        // Assert message received
    }
}
```

#### Performance Tests
Use JMH for benchmarks:

```java
@Benchmark
@BenchmarkMode(Mode.Throughput)
@OutputTimeUnit(TimeUnit.SECONDS)
public void benchmarkEnvelopeSerialization(Blackhole bh) {
    Envelope envelope = createTestEnvelope();
    bh.consume(envelope.serialize());
}
```

### Running Tests

```bash
# All tests
mvn test

# Specific test
mvn test -Dtest=EnvelopeTest

# Integration tests only
mvn verify

# With coverage
mvn test jacoco:report

# Benchmarks
mvn test -Pbenchmark
```

### Test Coverage Requirements

- **Overall**: Minimum 80%
- **Core classes**: Minimum 90%
- **Transport layer**: Minimum 85%
- **Examples**: Not required

## Submitting Changes

### Before Submitting

1. **Code quality**: Run all quality checks
2. **Tests**: Ensure all tests pass
3. **Documentation**: Update JavaDoc and README if needed
4. **Changelog**: Add entry to CHANGELOG.md
5. **Commits**: Follow commit message conventions

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Adding tests
- `chore`: Maintenance tasks

**Examples**:
```
feat(transport): add WebSocket peer implementation

Implement multiplexed WebSocket peer with auto-handshake protocol.
Supports both incoming and outgoing connections.

Closes #123
```

```
fix(core): correct envelope validation logic

Fixed issue where null capabilities caused NullPointerException.

Fixes #456
```

### Pull Request Process

1. **Create PR** with descriptive title and description
2. **Reference issues**: Link related issues
3. **Describe changes**: Explain what and why
4. **Add screenshots**: If UI changes
5. **Wait for review**: Address feedback
6. **Merge**: After approval and CI passes

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Checklist
- [ ] Tests added/updated
- [ ] Documentation updated
- [ ] Code follows style guide
- [ ] All tests pass
- [ ] Changelog updated

## Related Issues
Closes #123
```

## Release Process

### Version Numbering

We follow [Semantic Versioning](https://semver.org/):
- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes

### Release Steps

1. **Update version** in `pom.xml` and `build.gradle.kts`
2. **Update CHANGELOG.md** with release notes
3. **Create release branch**: `release/v1.0.0`
4. **Run full test suite**
5. **Create release tag**: `v1.0.0`
6. **Publish to Maven Central**
7. **Create GitHub release**

## Questions?

- **Issues**: https://github.com/hivellm/umicp/issues
- **Discussions**: https://github.com/hivellm/umicp/discussions
- **Email**: team@hivellm.org

Thank you for contributing! 🎉

