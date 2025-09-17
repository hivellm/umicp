# UMICP Documentation System

This directory contains the comprehensive documentation system for UMICP (Universal Matrix-based Inter-Component Protocol), built using the Dgeni documentation generation tool.

## Overview

The UMICP documentation system provides:

- **API Reference**: Complete C++ and TypeScript API documentation
- **User Guides**: Step-by-step installation, configuration, and usage guides
- **Examples**: Real-world code examples and integration patterns
- **Performance Benchmarks**: Detailed performance characteristics and optimization guides
- **Architecture Documentation**: System design and component interaction details

## Documentation Structure

```
docs/
├── dgeni.config.js          # Dgeni configuration
├── package.json             # Node.js dependencies and scripts
├── README.md               # This file
├── templates/              # HTML templates for generated docs
│   ├── api-index.template.html
│   ├── class.template.html
│   └── function.template.html
├── assets/                 # Static assets (CSS, JS, images)
│   ├── css/
│   │   └── docs.css        # Main stylesheet
│   ├── js/
│   │   └── docs.js         # Interactive functionality
│   └── images/
│       └── umicp-logo.svg  # UMICP logo
├── guides/                 # User guides and tutorials
│   ├── installation.md
│   ├── quick-start.md
│   ├── protocol-api.md
│   ├── configuration.md
│   └── examples.md
└── api/                    # Generated documentation output
    ├── index.html          # Main documentation page
    ├── classes/            # Class documentation
    ├── functions/          # Function documentation
    └── guides/             # Processed guides
```

## Getting Started

### Prerequisites

- Node.js 16+ and npm 8+
- Git (for cloning the repository)

### Installation

1. **Clone the repository** (if not already done):
   ```bash
   git clone https://github.com/umicp/umicp.git
   cd umicp/docs
   ```

2. **Install dependencies**:
   ```bash
   npm install
   ```

### Building Documentation

#### Development Build

```bash
# Build documentation once
npm run build

# Build and serve locally
npm run dev

# Watch for changes and rebuild automatically
npm run watch
```

#### Production Build

```bash
# Clean previous build and rebuild
npm run rebuild

# Build optimized version
NODE_ENV=production npm run build
```

### Serving Documentation

```bash
# Serve locally on http://localhost:8080
npm run serve

# Or use any other static server
python -m http.server 8080 -d ./api
```

## Documentation Features

### Dgeni-Powered Generation

The documentation system uses Dgeni, a powerful documentation generator that:

- **Extracts documentation** from C++ header files and source code
- **Processes TypeScript** bindings and generates type-safe documentation
- **Renders templates** using Nunjucks templating engine
- **Generates cross-references** between classes, functions, and guides
- **Creates searchable content** with full-text search capabilities

### Advanced Documentation Tags

The system supports comprehensive JSDoc-style tags for rich documentation:

#### Core Tags
- `@description` - Detailed description of the item
- `@param` - Parameter documentation with types
- `@returns` - Return value documentation
- `@throws` - Exception documentation
- `@example` - Code examples with syntax highlighting

#### UMICP-Specific Tags
- `@performance` - Performance characteristics and benchmarks
- `@complexity` - Algorithmic complexity (Big O notation)
- `@simd` - SIMD optimization information
- `@security` - Security considerations and best practices
- `@threadsafe` - Thread safety guarantees
- `@nodejs` - Node.js binding availability
- `@tested` - Test coverage information
- `@group` - Logical grouping of related items

#### Example Documentation Block

```cpp
/**
 * @class MatrixOps
 * @description High-performance matrix operations with SIMD optimization
 * 
 * Provides optimized implementations of common matrix operations including
 * multiplication, addition, and specialized ML operations like dot products
 * and cosine similarity calculations.
 * 
 * @group Matrix Operations
 * @simd Optimized with AVX2 and SSE4.2 instructions
 * @threadsafe All operations are thread-safe
 * @performance >12 GFLOPS on modern CPUs
 * @tested 100% test coverage with comprehensive benchmarks
 * @nodejs Available through Node.js bindings
 * 
 * @example
 * ```cpp
 * // Matrix multiplication example
 * std::vector<float> a(1000), b(1000), result(1000);
 * MatrixOps::multiply(a.data(), b.data(), result.data(), 10, 10, 10);
 * ```
 */
class MatrixOps {
    /**
     * @method multiply
     * @description Performs matrix multiplication with SIMD optimization
     * 
     * @param {float*} a - First matrix (row-major order)
     * @param {float*} b - Second matrix (row-major order)  
     * @param {float*} result - Output matrix (row-major order)
     * @param {size_t} rows_a - Number of rows in matrix A
     * @param {size_t} cols_a - Number of columns in matrix A
     * @param {size_t} cols_b - Number of columns in matrix B
     * @returns {Result<void>} Success or error result
     * 
     * @complexity O(n³) for square matrices
     * @performance 12.5 GFLOPS on Intel i7-10700K
     * @simd Uses AVX2 for 8-way parallel operations
     * @threadsafe
     * 
     * @throws {std::invalid_argument} If matrix dimensions are incompatible
     * 
     * @example
     * ```cpp
     * std::vector<float> a(100), b(100), result(100);
     * auto res = MatrixOps::multiply(a.data(), b.data(), result.data(), 10, 10, 10);
     * assert(res.is_success());
     * ```
     */
    static Result<void> multiply(float* a, float* b, float* result,
                               size_t rows_a, size_t cols_a, size_t cols_b);
};
```

### Template System

#### Available Templates

1. **api-index.template.html** - Main documentation index page
2. **class.template.html** - Class documentation pages
3. **function.template.html** - Function documentation pages
4. **guide.template.html** - User guide pages
5. **common.template.html** - Shared template components

#### Template Features

- **Responsive design** that works on desktop and mobile
- **Dark/light theme** support with automatic detection
- **Interactive navigation** with smooth scrolling and highlighting
- **Code syntax highlighting** using highlight.js
- **Search functionality** with instant results
- **Cross-references** between related documentation items

### Styling and Theming

The documentation uses a modern, professional design with:

- **CSS Custom Properties** for easy theming
- **Mobile-first responsive design**
- **High contrast** for accessibility
- **Smooth animations** and transitions
- **Professional typography** with Inter and JetBrains Mono fonts

#### Color Scheme

```css
/* Light Theme */
--primary-color: #2563eb;
--bg-primary: #ffffff;
--text-primary: #0f172a;

/* Dark Theme */
--primary-color: #3b82f6;
--bg-primary: #0f172a;
--text-primary: #f8fafc;
```

## Writing Documentation

### For C++ Code

Use JSDoc-style comments above classes, functions, and methods:

```cpp
/**
 * @brief Brief description (legacy JSDoc)
 * @description Detailed description (Dgeni preferred)
 * @param name Description
 * @returns Description
 * @throws Exception conditions
 * @example Code example
 * @performance Performance notes
 * @threadsafe Thread safety info
 */
```

### For Guides

Create Markdown files in the `guides/` directory with front matter:

```markdown
---
title: "Guide Title"
description: "Guide description"
group: "Getting Started"
order: 1
---

# Guide Content

Regular Markdown content with code examples...
```

### For Examples

Include comprehensive examples with:

- **Clear problem statement**
- **Complete, runnable code**
- **Expected output**
- **Explanation of key concepts**
- **Links to related documentation**

## Development Workflow

### Adding New Documentation

1. **Create or update** source files with documentation comments
2. **Run documentation build** to see changes
3. **Test locally** using the development server
4. **Commit changes** following the project's Git workflow

### Quality Checklist

Before committing documentation changes:

- [ ] All public APIs have comprehensive documentation
- [ ] Examples are complete and tested
- [ ] Links work correctly
- [ ] Spelling and grammar are correct
- [ ] Code examples follow project style guidelines
- [ ] Performance claims are backed by benchmarks
- [ ] Security considerations are documented

### Automated Checks

The documentation system includes automated checks for:

- **Link validation** - Ensures all internal links work
- **Spelling check** - Catches common spelling errors
- **Code example validation** - Verifies code examples compile
- **Template validation** - Ensures templates render correctly

## Configuration

### Dgeni Configuration

The main configuration is in `dgeni.config.js`:

```javascript
// Key configuration sections:
- Source file patterns
- Output directory settings  
- Template configuration
- Processing pipeline
- Custom tags and processors
```

### Build Scripts

Available npm scripts:

- `npm run build` - Build documentation
- `npm run serve` - Serve locally
- `npm run dev` - Build and serve
- `npm run watch` - Watch and rebuild
- `npm run clean` - Clean output
- `npm run rebuild` - Clean and rebuild
- `npm run lint` - Lint configuration

## Deployment

### GitHub Pages

The documentation can be automatically deployed to GitHub Pages:

```yaml
# .github/workflows/docs.yml
name: Deploy Documentation
on:
  push:
    branches: [main]
jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-node@v3
        with:
          node-version: '18'
      - run: cd docs && npm ci
      - run: cd docs && npm run build
      - uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./docs/api
```

### Custom Hosting

For custom hosting, build the documentation and serve the `api/` directory:

```bash
npm run build
# Upload ./api directory to your web server
```

## Troubleshooting

### Common Issues

1. **Build fails with module not found**
   ```bash
   rm -rf node_modules package-lock.json
   npm install
   ```

2. **Templates not rendering correctly**
   - Check template syntax in `templates/` directory
   - Verify template paths in `dgeni.config.js`

3. **Code examples not highlighting**
   - Ensure language is specified in code blocks
   - Check highlight.js configuration

4. **Links broken in generated docs**
   - Verify file paths in source documentation
   - Check `computePathsProcessor` configuration

### Debug Mode

Enable debug output:

```bash
DEBUG=dgeni:* npm run build
```

### Getting Help

- **Issues**: Report bugs on [GitHub Issues](https://github.com/umicp/umicp/issues)
- **Discussions**: Ask questions in [GitHub Discussions](https://github.com/umicp/umicp/discussions)
- **Documentation**: See [Dgeni Documentation](https://github.com/angular/dgeni)

## Contributing

We welcome contributions to improve the documentation:

1. **Fork the repository**
2. **Create a feature branch**
3. **Make your changes**
4. **Test the documentation build**
5. **Submit a pull request**

See [CONTRIBUTING.md](../CONTRIBUTING.md) for detailed guidelines.

## License

The UMICP documentation system is licensed under the MIT License. See [LICENSE](../LICENSE) for details.
