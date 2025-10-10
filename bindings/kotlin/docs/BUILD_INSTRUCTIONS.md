# UMICP Kotlin SDK - Build Instructions

**Version**: 0.1.2  
**Date**: 2025-10-10

---

## 📋 Prerequisites

### Required
- **JDK**: 17 or higher
- **Gradle**: 8.5+ (or use wrapper)

### Verify Installation
```bash
java -version   # Should show Java 17+
```

---

## 🔧 Building the SDK

### Option 1: Using Gradle Wrapper (Recommended)

```bash
# Navigate to kotlin directory
cd umicp/bindings/kotlin

# Build (downloads Gradle if needed)
./gradlew build

# Run tests
./gradlew test

# Run with coverage
./gradlew test jacocoTestReport

# Clean build
./gradlew clean build
```

### Option 2: Using Installed Gradle

```bash
cd umicp/bindings/kotlin

gradle build
gradle test
```

---

## 🧪 Running Tests

### All Tests
```bash
./gradlew test
```

### Specific Test Class
```bash
./gradlew test --tests "com.hivellm.umicp.core.EnvelopeTest"
./gradlew test --tests "com.hivellm.umicp.compression.CompressionTest"
./gradlew test --tests "com.hivellm.umicp.transport.HttpServerTest"
```

### With Coverage Report
```bash
./gradlew test jacocoTestReport

# View report
open build/reports/jacoco/test/html/index.html
```

### Integration Tests
```bash
# Enable integration tests
./gradlew test -Denable.integration.tests=true
```

---

## 📖 Running Examples

### Basic Example
```bash
./gradlew run --args="com.hivellm.umicp.examples.BasicExample"
```

### Matrix Example
```bash
./gradlew run --args="com.hivellm.umicp.examples.MatrixExample"
```

### WebSocket Example
```bash
./gradlew run --args="com.hivellm.umicp.examples.WebSocketExample"
```

### Service Discovery Example
```bash
./gradlew run --args="com.hivellm.umicp.examples.ServiceDiscoveryExample"
```

### Compression Example (with LZ4)
```bash
./gradlew run --args="com.hivellm.umicp.examples.CompressionExample"
```

### HTTP Client Example
```bash
./gradlew run --args="com.hivellm.umicp.examples.HttpExample"
```

### HTTP Server Example
```bash
./gradlew run --args="com.hivellm.umicp.examples.HttpServerExample"
```

---

## 📦 Publishing

### To Maven Local
```bash
./gradlew publishToMavenLocal
```

### To Maven Central (requires credentials)
```bash
./gradlew publish
```

---

## 🧹 Cleaning

### Clean Build Directory
```bash
./gradlew clean
```

### Deep Clean
```bash
./gradlew clean cleanBuildCache
rm -rf build .gradle
```

---

## 📊 Other Tasks

### Generate Documentation
```bash
./gradlew dokkaHtml

# View docs
open build/docs/index.html
```

### List All Tasks
```bash
./gradlew tasks
```

### Check Dependencies
```bash
./gradlew dependencies
```

### Check for Updates
```bash
./gradlew dependencyUpdates
```

---

## ⚠️ Troubleshooting

### Gradle Daemon Issues
```bash
# Stop daemon
./gradlew --stop

# Run without daemon
./gradlew build --no-daemon
```

### Out of Memory
```bash
# Increase memory in gradle.properties
org.gradle.jvmargs=-Xmx4g -XX:MaxMetaspaceSize=1g
```

### Permission Issues (Linux/Mac)
```bash
chmod +x gradlew
./gradlew build
```

### Clean Start
```bash
./gradlew clean
rm -rf build .gradle
./gradlew build --refresh-dependencies
```

---

## 🚀 Quick Start

```bash
# Clone repo
git clone https://github.com/hivellm/umicp.git
cd umicp/bindings/kotlin

# Build and test
./gradlew build test

# Run example
./gradlew run --args="com.hivellm.umicp.examples.BasicExample"

# Generate docs
./gradlew dokkaHtml
```

---

## ✅ Expected Build Output

```
> Task :compileKotlin
> Task :compileJava NO-SOURCE
> Task :processResources NO-SOURCE
> Task :classes
> Task :jar
> Task :assemble
> Task :compileTestKotlin
> Task :processTestResources NO-SOURCE
> Task :testClasses
> Task :test

BUILD SUCCESSFUL in 45s
8 actionable tasks: 8 executed
```

---

## 📈 Build Performance

| Task | Time | Notes |
|------|------|-------|
| **compileKotlin** | ~15s | First build |
| **compileKotlin** | ~2s | Incremental |
| **test** | ~8s | 140+ tests |
| **jar** | ~1s | Packaging |
| **Total** | ~25s | First build |
| **Total** | ~5s | Incremental |

---

**Gradle Version**: 8.5  
**Kotlin Version**: 1.9.21  
**JVM Target**: 17

