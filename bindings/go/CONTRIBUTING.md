# Contributing to UMICP Go Bindings

Thank you for your interest in contributing to the UMICP Go bindings! This document provides guidelines and instructions for contributing.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)
- [Pull Request Process](#pull-request-process)

---

## 🤝 Code of Conduct

This project adheres to the [HiveLLM Code of Conduct](../../CONTRIBUTING.md). By participating, you are expected to uphold this code.

---

## 🚀 Getting Started

### Prerequisites

- **Go 1.21+** installed
- **Git** for version control
- Familiarity with Go programming
- Understanding of UMICP protocol (see [docs/](docs/))

### Quick Start

1. **Fork the repository**
2. **Clone your fork**:
   ```bash
   git clone https://github.com/YOUR_USERNAME/umicp.git
   cd umicp/bindings/go
   ```
3. **Install dependencies**:
   ```bash
   go mod download
   ```
4. **Run tests**:
   ```bash
   go test ./...
   ```

---

## 🛠️ Development Setup

### 1. Install Go Tools

```bash
# Install golangci-lint
go install github.com/golangci/golangci-lint/cmd/golangci-lint@latest

# Install test coverage tools
go install github.com/axw/gocov/gocov@latest
go install github.com/matm/gocov-html/cmd/gocov-html@latest
```

### 2. Configure Your Editor

We recommend using VS Code with the Go extension, or GoLand.

**VS Code settings** (`.vscode/settings.json`):
```json
{
  "go.useLanguageServer": true,
  "go.lintTool": "golangci-lint",
  "go.lintOnSave": "workspace",
  "go.formatTool": "goimports",
  "editor.formatOnSave": true
}
```

### 3. Verify Setup

```bash
go test ./...
go vet ./...
golangci-lint run
```

---

## 💡 How to Contribute

### Areas for Contribution

See [docs/IMPLEMENTATION_ROADMAP.md](docs/IMPLEMENTATION_ROADMAP.md) for detailed tasks.

**High Priority (🔴 CRITICAL)**:
- Phase 1: Foundation Layer (Envelope, Frame, Matrix)
- Phase 2: WebSocket Transport (Client + Server)
- Phase 3: Multiplexed Peer Architecture

**Medium Priority (🟡 HIGH)**:
- Phase 5: Testing Infrastructure
- Bug fixes and improvements

**Lower Priority (🟢 MEDIUM)**:
- Phase 4: HTTP/2 Transport
- Phase 6: Documentation and Examples

### Finding Issues

- Check [GitHub Issues](https://github.com/hivellm/umicp/issues)
- Look for `good-first-issue` label
- Look for `help-wanted` label
- Comment on an issue to claim it

---

## 📝 Coding Standards

### Go Style Guide

Follow the official [Effective Go](https://go.dev/doc/effective_go) guidelines.

### Key Principles

1. **Simplicity**: Write simple, readable code
2. **Idiomatic Go**: Use Go patterns and conventions
3. **Error Handling**: Always handle errors explicitly
4. **Context**: Use `context.Context` for cancellation
5. **Testing**: Write tests for all new code

### Code Organization

```go
// Package declaration
package umicp

// Imports (grouped: stdlib, external, internal)
import (
    "context"
    "fmt"
    
    "github.com/google/uuid"
    
    "github.com/hivellm/umicp-go/pkg/transport"
)

// Constants
const (
    DefaultTimeout = 10 * time.Second
)

// Types
type Envelope struct {
    // Fields with comments
    From string `json:"from"` // Sender ID
}

// Constructor
func NewEnvelope() *EnvelopeBuilder {
    return &EnvelopeBuilder{
        envelope: &Envelope{},
    }
}

// Methods (receiver, name, parameters, returns)
func (e *Envelope) Validate() error {
    if e.From == "" {
        return ErrMissingFrom
    }
    return nil
}
```

### Naming Conventions

- **Packages**: lowercase, single word (e.g., `umicp`, `transport`)
- **Files**: lowercase with underscores (e.g., `envelope.go`, `websocket_client.go`)
- **Types**: PascalCase (e.g., `Envelope`, `WebSocketClient`)
- **Functions**: PascalCase for exported, camelCase for private
- **Constants**: PascalCase or UPPER_CASE for enums
- **Variables**: camelCase

### Comments

- **Package**: Document purpose at top of main file
- **Exported types/functions**: Must have doc comments
- **Complex logic**: Add inline comments explaining why

```go
// Envelope represents a UMICP message container with metadata.
// It provides methods for serialization, validation, and manipulation.
type Envelope struct {
    From string `json:"from"`
}

// NewEnvelope creates a new envelope builder with default values.
func NewEnvelope() *EnvelopeBuilder {
    return &EnvelopeBuilder{
        envelope: &Envelope{
            MessageID: uuid.New().String(),
        },
    }
}
```

---

## 🧪 Testing Guidelines

### Test Structure

```go
package umicp_test

import (
    "testing"
    
    "github.com/stretchr/testify/assert"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func TestEnvelope_Validate(t *testing.T) {
    tests := []struct {
        name    string
        setup   func() *umicp.Envelope
        wantErr bool
    }{
        {
            name: "valid envelope",
            setup: func() *umicp.Envelope {
                env, _ := umicp.NewEnvelope().
                    From("a").
                    To("b").
                    Build()
                return env
            },
            wantErr: false,
        },
        {
            name: "missing from field",
            setup: func() *umicp.Envelope {
                return &umicp.Envelope{To: "b"}
            },
            wantErr: true,
        },
    }
    
    for _, tt := range tests {
        t.Run(tt.name, func(t *testing.T) {
            env := tt.setup()
            err := env.Validate()
            
            if tt.wantErr {
                assert.Error(t, err)
            } else {
                assert.NoError(t, err)
            }
        })
    }
}
```

### Test Coverage

- **Target**: 90%+ coverage for all packages
- **Unit tests**: Test individual functions/methods
- **Integration tests**: Test component interactions
- **E2E tests**: Test complete workflows

### Running Tests

```bash
# All tests
go test ./...

# Specific package
go test ./pkg/umicp/

# With coverage
go test -cover ./...

# Verbose
go test -v ./...

# Race detector
go test -race ./...

# Coverage report
go test -coverprofile=coverage.out ./...
go tool cover -html=coverage.out
```

---

## 📖 Documentation

### Code Documentation

- Use `godoc` format for all exported items
- Include examples in doc comments when helpful
- Keep documentation up-to-date with code changes

### README Updates

Update relevant README files when:
- Adding new features
- Changing APIs
- Adding examples

### Documentation Structure

```
docs/
├── INDEX.md                      # Documentation index
├── EXECUTIVE_SUMMARY.md          # Overview
├── IMPLEMENTATION_ROADMAP.md     # Implementation plan
├── COMPARISON.md                 # TypeScript comparison
└── GETTING_STARTED.md            # Tutorial
```

---

## 🔄 Pull Request Process

### Before Submitting

1. **Create a branch**:
   ```bash
   git checkout -b feature/my-feature
   ```

2. **Make your changes**:
   - Follow coding standards
   - Write tests
   - Update documentation

3. **Run checks**:
   ```bash
   go test ./...
   go vet ./...
   golangci-lint run
   go fmt ./...
   ```

4. **Commit changes**:
   ```bash
   git add .
   git commit -m "feat: add envelope validation"
   ```

### Commit Message Format

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `test`: Test changes
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `chore`: Build/tooling changes

**Examples**:
```
feat(envelope): add builder pattern for envelope creation

- Implement EnvelopeBuilder with fluent API
- Add validation in Build() method
- Update tests and documentation

Closes #123
```

### Submitting PR

1. **Push to your fork**:
   ```bash
   git push origin feature/my-feature
   ```

2. **Create Pull Request**:
   - Go to GitHub and create PR
   - Fill in the PR template
   - Link related issues

3. **PR Template**:
   ```markdown
   ## Description
   Brief description of changes
   
   ## Type of Change
   - [ ] Bug fix
   - [ ] New feature
   - [ ] Documentation update
   - [ ] Performance improvement
   
   ## Checklist
   - [ ] Tests pass locally
   - [ ] Added tests for new code
   - [ ] Updated documentation
   - [ ] Followed coding standards
   - [ ] No breaking changes
   
   ## Related Issues
   Closes #123
   ```

### Review Process

1. **Automated checks**: CI/CD must pass
2. **Code review**: At least one approval required
3. **Address feedback**: Make requested changes
4. **Merge**: Maintainer will merge after approval

---

## 🐛 Reporting Bugs

### Bug Report Template

```markdown
**Describe the bug**
Clear description of the bug

**To Reproduce**
Steps to reproduce:
1. Go to '...'
2. Click on '....'
3. See error

**Expected behavior**
What you expected to happen

**Actual behavior**
What actually happened

**Environment**
- Go version: [e.g., 1.21.0]
- OS: [e.g., Ubuntu 22.04]
- UMICP version: [e.g., v1.0.0]

**Additional context**
Any other relevant information
```

---

## 💬 Questions and Support

- **Questions**: Use [GitHub Discussions](https://github.com/hivellm/umicp/discussions)
- **Bugs**: Use [GitHub Issues](https://github.com/hivellm/umicp/issues)
- **Chat**: Join our community chat (if available)

---

## 📄 License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

## 🙏 Thank You!

Thank you for contributing to UMICP Go bindings! Your contributions help make this project better for everyone.

---

**Last Updated**: 2025-10-10  
**Version**: 1.0  
**Maintainer**: HiveLLM AI Collaborative Team

