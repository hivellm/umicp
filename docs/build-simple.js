#!/usr/bin/env node

/**
 * Simple UMICP Documentation Builder
 * A simplified version of the Dgeni documentation system for testing
 */

const fs = require('fs');
const path = require('path');

// Configuration
const config = {
    sourceDir: path.join(__dirname, 'guides'),
    templateDir: path.join(__dirname, 'templates'),
    outputDir: path.join(__dirname, 'api'),
    assetsDir: path.join(__dirname, 'assets')
};

// Ensure output directory exists
function ensureDir(dir) {
    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true });
    }
}

// Simple template processor
function processTemplate(templatePath, data) {
    let template = fs.readFileSync(templatePath, 'utf8');

    // Simple variable replacement
    template = template.replace(/\{\{\s*(\w+(?:\.\w+)*)\s*\}\}/g, (match, path) => {
        const value = getNestedProperty(data, path);
        return value !== undefined ? value : match;
    });

    return template;
}

// Get nested property from object
function getNestedProperty(obj, path) {
    return path.split('.').reduce((current, key) => {
        return current && current[key] !== undefined ? current[key] : undefined;
    }, obj);
}

// Process Markdown to simple HTML
function markdownToHtml(markdown) {
    return markdown
        // Headers
        .replace(/^### (.*$)/gim, '<h3>$1</h3>')
        .replace(/^## (.*$)/gim, '<h2>$1</h2>')
        .replace(/^# (.*$)/gim, '<h1>$1</h1>')
        // Code blocks
        .replace(/```(\w+)?\n([\s\S]*?)```/g, '<pre><code class="language-$1">$2</code></pre>')
        // Inline code
        .replace(/`([^`]+)`/g, '<code>$1</code>')
        // Bold and italic
        .replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>')
        .replace(/\*([^*]+)\*/g, '<em>$1</em>')
        // Links
        .replace(/\[([^\]]+)\]\(([^)]+)\)/g, '<a href="$2">$1</a>')
        // Paragraphs
        .replace(/\n\n/g, '</p><p>')
        .replace(/^(?!<[h|p|u|o|l|d|c])/gm, '<p>')
        .replace(/(?<!>)$/gm, '</p>')
        // Clean up
        .replace(/<p><\/p>/g, '')
        .replace(/<p>(<[h|u|o|d|c])/g, '$1')
        .replace(/(<\/[h|u|o|d|c][^>]*>)<\/p>/g, '$1');
}

// Build documentation
function buildDocs() {
    console.log('🚀 Building UMICP Documentation...');

    // Ensure directories exist
    ensureDir(config.outputDir);
    ensureDir(path.join(config.outputDir, 'guides'));
    ensureDir(path.join(config.outputDir, 'assets'));

    // Copy assets
    console.log('📄 Copying assets...');
    const assetsSrc = config.assetsDir;
    const assetsDest = path.join(config.outputDir, 'assets');

    if (fs.existsSync(assetsSrc)) {
        copyRecursive(assetsSrc, assetsDest);
    }

    // Process guides
    console.log('📖 Processing guides...');
    const guidesDir = config.sourceDir;

    if (fs.existsSync(guidesDir)) {
        const guides = fs.readdirSync(guidesDir)
            .filter(file => file.endsWith('.md'))
            .map(file => {
                const content = fs.readFileSync(path.join(guidesDir, file), 'utf8');
                const name = path.basename(file, '.md');

                // Extract title from first heading
                const titleMatch = content.match(/^#\s+(.+)$/m);
                const title = titleMatch ? titleMatch[1] : name;

                return {
                    name,
                    title,
                    content: markdownToHtml(content),
                    filename: file
                };
            });

        // Create index page
        console.log('🏠 Creating index page...');
        const indexData = {
            title: 'UMICP Documentation',
            guides: guides,
            apiStructure: {
                classes: {},
                functions: {},
                guides: guides.reduce((acc, guide) => {
                    acc[guide.name] = guide;
                    return acc;
                }, {})
            }
        };

        const indexTemplate = path.join(config.templateDir, 'api-index.template.html');
        if (fs.existsSync(indexTemplate)) {
            const indexHtml = processTemplate(indexTemplate, { doc: indexData });
            fs.writeFileSync(path.join(config.outputDir, 'index.html'), indexHtml);
        } else {
            // Create simple index if template doesn't exist
            const simpleIndex = createSimpleIndex(indexData);
            fs.writeFileSync(path.join(config.outputDir, 'index.html'), simpleIndex);
        }

        // Process individual guides
        guides.forEach(guide => {
            const guideHtml = createSimpleGuide(guide);
            fs.writeFileSync(path.join(config.outputDir, 'guides', `${guide.name}.html`), guideHtml);
        });
    }

    console.log('✅ Documentation build complete!');
    console.log(`📁 Output directory: ${config.outputDir}`);
    console.log('🌐 Open index.html in your browser to view the documentation');
}

// Copy files recursively
function copyRecursive(src, dest) {
    ensureDir(dest);
    const entries = fs.readdirSync(src, { withFileTypes: true });

    for (let entry of entries) {
        const srcPath = path.join(src, entry.name);
        const destPath = path.join(dest, entry.name);

        if (entry.isDirectory()) {
            copyRecursive(srcPath, destPath);
        } else {
            fs.copyFileSync(srcPath, destPath);
        }
    }
}

// Create simple index page
function createSimpleIndex(data) {
    return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>${data.title}</title>
    <link rel="stylesheet" href="assets/css/docs.css">
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; }
        .container { max-width: 1200px; margin: 0 auto; padding: 2rem; }
        .guide-list { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 1rem; margin-top: 2rem; }
        .guide-card { border: 1px solid #e2e8f0; border-radius: 0.5rem; padding: 1.5rem; background: #f8fafc; }
        .guide-card h3 { margin-top: 0; color: #2563eb; }
        .guide-card a { text-decoration: none; color: inherit; }
        .guide-card:hover { box-shadow: 0 4px 6px -1px rgb(0 0 0 / 0.1); }
        .header { text-align: center; margin-bottom: 3rem; }
        .logo { font-size: 2rem; font-weight: bold; color: #2563eb; }
    </style>
</head>
<body>
    <div class="container">
        <header class="header">
            <div class="logo">UMICP Documentation</div>
            <p>Universal Matrix-based Inter-Component Protocol</p>
        </header>

        <main>
            <h2>Documentation Guides</h2>
            <div class="guide-list">
                ${data.guides.map(guide => `
                    <div class="guide-card">
                        <a href="guides/${guide.name}.html">
                            <h3>${guide.title}</h3>
                            <p>Click to read the complete guide</p>
                        </a>
                    </div>
                `).join('')}
            </div>
        </main>

        <footer style="margin-top: 3rem; padding-top: 2rem; border-top: 1px solid #e2e8f0; text-align: center; color: #64748b;">
            <p>&copy; 2025 UMICP Project. Documentation built on ${new Date().toLocaleString()}</p>
        </footer>
    </div>

    <script src="assets/js/docs.js"></script>
</body>
</html>`;
}

// Create simple guide page
function createSimpleGuide(guide) {
    return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>${guide.title} - UMICP Documentation</title>
    <link rel="stylesheet" href="../assets/css/docs.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/default.min.css">
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; line-height: 1.6; }
        .container { max-width: 800px; margin: 0 auto; padding: 2rem; }
        .breadcrumb { margin-bottom: 2rem; color: #64748b; }
        .breadcrumb a { color: #2563eb; text-decoration: none; }
        .content { color: #334155; }
        .content h1, .content h2, .content h3 { color: #0f172a; }
        .content code { background: #f1f5f9; padding: 0.2em 0.4em; border-radius: 0.25rem; font-size: 0.9em; }
        .content pre { background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 0.5rem; padding: 1rem; overflow-x: auto; }
        .content pre code { background: none; padding: 0; }
        .content table { border-collapse: collapse; width: 100%; margin: 1rem 0; }
        .content th, .content td { border: 1px solid #e2e8f0; padding: 0.75rem; text-align: left; }
        .content th { background: #f8fafc; font-weight: 600; }
    </style>
</head>
<body>
    <div class="container">
        <nav class="breadcrumb">
            <a href="../index.html">Home</a> / <a href="../index.html">Guides</a> / ${guide.title}
        </nav>

        <main class="content">
            ${guide.content}
        </main>

        <footer style="margin-top: 3rem; padding-top: 2rem; border-top: 1px solid #e2e8f0; text-align: center; color: #64748b;">
            <p><a href="../index.html">&larr; Back to Documentation Home</a></p>
        </footer>
    </div>

    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
    <script src="../assets/js/docs.js"></script>
    <script>hljs.highlightAll();</script>
</body>
</html>`;
}

// Run the build
if (require.main === module) {
    buildDocs();
}

module.exports = { buildDocs, config };
