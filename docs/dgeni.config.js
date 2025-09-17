/**
 * @fileoverview Dgeni Documentation Configuration for UMICP
 * @description Configuration file for generating comprehensive API documentation
 * using the Dgeni documentation generation tool.
 *
 * This configuration processes C++ source files, TypeScript bindings,
 * and markdown guides to create a unified documentation site.
 *
 * @version 1.0.0
 * @since 2025-01-10
 * @author UMICP Development Team
 */

const Package = require('dgeni').Package;
const jsdocPackage = require('dgeni-packages/jsdoc');
const nunjucksPackage = require('dgeni-packages/nunjucks');
const typescriptPackage = require('dgeni-packages/typescript');

/**
 * @description Main Dgeni package configuration for UMICP documentation
 * @type {Package}
 */
module.exports = new Package('umicp-docs', [
  jsdocPackage,
  nunjucksPackage,
  typescriptPackage
])

// Configure source files and paths
.config(function(readFilesProcessor, writeFilesProcessor) {

  /**
   * @description Source file patterns for documentation extraction
   * @type {Array<string>}
   */
  readFilesProcessor.sourceFiles = [
    // C++ header files with documentation
    {
      include: '../cpp/include/**/*.h',
      exclude: '../cpp/include/**/internal/**',
      basePath: '../cpp/include'
    },

    // C++ source files with implementation docs
    {
      include: '../cpp/src/**/*.cpp',
      exclude: '../cpp/src/**/test/**',
      basePath: '../cpp/src'
    },

    // TypeScript bindings
    {
      include: '../bindings/**/*.ts',
      exclude: '../bindings/**/*.spec.ts',
      basePath: '../bindings'
    },

    // Documentation guides and tutorials
    {
      include: './**/*.md',
      exclude: './api/**',
      basePath: '.'
    },

    // Examples and code samples
    {
      include: '../examples/**/*.{cpp,ts,js}',
      basePath: '../examples'
    }
  ];

  /**
   * @description Output configuration for generated documentation
   */
  writeFilesProcessor.outputFolder = './api';
})

// Configure TypeScript processing
.config(function(parseTagsProcessor, extractTypeScriptProcessor) {

  /**
   * @description TypeScript compiler options for documentation
   */
  extractTypeScriptProcessor.tsConfigPath = '../bindings/tsconfig.json';

  /**
   * @description Custom JSDoc tags for UMICP documentation
   */
  parseTagsProcessor.tagDefinitions = parseTagsProcessor.tagDefinitions.concat([

    // Performance-related tags
    { name: 'performance', docProperty: 'performance', multi: true },
    { name: 'complexity', docProperty: 'complexity' },
    { name: 'simd', docProperty: 'simdOptimized', transforms: [require('dgeni-packages/base/transforms/trimWhitespace')] },

    // Security and safety tags
    { name: 'security', docProperty: 'security', multi: true },
    { name: 'threadsafe', docProperty: 'threadSafe', transforms: [function() { return true; }] },
    { name: 'reentrant', docProperty: 'reentrant', transforms: [function() { return true; }] },

    // UMICP-specific tags
    { name: 'protocol', docProperty: 'protocolVersion' },
    { name: 'transport', docProperty: 'transportType' },
    { name: 'serialization', docProperty: 'serializationFormat', multi: true },
    { name: 'matrix', docProperty: 'matrixOperation', transforms: [require('dgeni-packages/base/transforms/trimWhitespace')] },

    // Quality and testing tags
    { name: 'tested', docProperty: 'tested', transforms: [function() { return true; }] },
    { name: 'coverage', docProperty: 'testCoverage' },
    { name: 'benchmark', docProperty: 'benchmarked', transforms: [function() { return true; }] },

    // Integration and compatibility
    { name: 'nodejs', docProperty: 'nodeJsCompatible', transforms: [function() { return true; }] },
    { name: 'binding', docProperty: 'bindingInfo', multi: true },

    // Documentation metadata
    { name: 'group', docProperty: 'group' },
    { name: 'order', docProperty: 'order', transforms: [function(doc, tag) { return parseInt(tag.description, 10); }] }
  ]);
})

// Configure template processing
.config(function(templateFinder, templateEngine) {

  /**
   * @description Template paths for different document types
   */
  templateFinder.templateFolders = [
    './templates'
  ];

  /**
   * @description Template patterns for different content types
   */
  templateFinder.templatePatterns = [
    '${ doc.template }',
    '${ doc.id }.${ doc.docType }.template.html',
    '${ doc.id }.template.html',
    '${ doc.docType }.template.html',
    'common.template.html'
  ];

  /**
   * @description Nunjucks template engine configuration
   */
  templateEngine.config.tags = {
    blockStart: '{%',
    blockEnd: '%}',
    variableStart: '{{',
    variableEnd: '}}',
    commentStart: '{#',
    commentEnd: '#}'
  };
})

// Configure processors for C++ documentation
.processor('cpp-extractor', function() {
  return {
    name: 'cpp-extractor',
    description: 'Extract documentation from C++ files',
    runAfter: ['files-read'],
    runBefore: ['processing-docs'],

    /**
     * @description Process C++ files to extract documentation
     * @param {Array} docs - Array of document objects
     * @returns {Array} Processed documents
     */
    process: function(docs) {
      const cppDocs = docs.filter(doc => doc.fileInfo.extension === 'h' || doc.fileInfo.extension === 'cpp');

      cppDocs.forEach(doc => {
        // Extract C++ class and function documentation
        this.extractCppDocumentation(doc);
      });

      return docs;
    },

    /**
     * @description Extract C++ documentation from source content
     * @param {Object} doc - Document object
     */
    extractCppDocumentation: function(doc) {
      const content = doc.content;

      // Extract class documentation
      const classMatches = content.match(/\/\*\*[\s\S]*?\*\/[\s]*class\s+(\w+)/g);
      if (classMatches) {
        doc.classes = classMatches.map(match => this.parseClassDoc(match));
      }

      // Extract function documentation
      const functionMatches = content.match(/\/\*\*[\s\S]*?\*\/[\s]*[a-zA-Z_][\w\s\*&<>:,]*\s+(\w+)\s*\(/g);
      if (functionMatches) {
        doc.functions = functionMatches.map(match => this.parseFunctionDoc(match));
      }
    },

    /**
     * @description Parse class documentation block
     * @param {string} classBlock - Raw class documentation block
     * @returns {Object} Parsed class information
     */
    parseClassDoc: function(classBlock) {
      const nameMatch = classBlock.match(/class\s+(\w+)/);
      const docMatch = classBlock.match(/\/\*\*([\s\S]*?)\*\//);

      return {
        name: nameMatch ? nameMatch[1] : 'Unknown',
        documentation: docMatch ? this.parseDocComment(docMatch[1]) : {},
        type: 'class'
      };
    },

    /**
     * @description Parse function documentation block
     * @param {string} functionBlock - Raw function documentation block
     * @returns {Object} Parsed function information
     */
    parseFunctionDoc: function(functionBlock) {
      const nameMatch = functionBlock.match(/(\w+)\s*\(/);
      const docMatch = functionBlock.match(/\/\*\*([\s\S]*?)\*\//);

      return {
        name: nameMatch ? nameMatch[1] : 'Unknown',
        documentation: docMatch ? this.parseDocComment(docMatch[1]) : {},
        type: 'function'
      };
    },

    /**
     * @description Parse JSDoc-style comment content
     * @param {string} comment - Raw comment content
     * @returns {Object} Parsed documentation object
     */
    parseDocComment: function(comment) {
      const lines = comment.split('\n').map(line => line.replace(/^\s*\*\s?/, '').trim());
      const doc = { description: '', tags: {} };

      let currentTag = null;
      let currentContent = [];

      lines.forEach(line => {
        const tagMatch = line.match(/^@(\w+)(?:\s+(.*))?/);

        if (tagMatch) {
          // Save previous tag content
          if (currentTag) {
            doc.tags[currentTag] = currentContent.join(' ').trim();
          }

          // Start new tag
          currentTag = tagMatch[1];
          currentContent = tagMatch[2] ? [tagMatch[2]] : [];
        } else if (currentTag) {
          // Continue current tag content
          currentContent.push(line);
        } else if (line) {
          // Main description
          doc.description += (doc.description ? ' ' : '') + line;
        }
      });

      // Save last tag
      if (currentTag) {
        doc.tags[currentTag] = currentContent.join(' ').trim();
      }

      return doc;
    }
  };
})

// Configure API documentation processor
.processor('api-docs', function() {
  return {
    name: 'api-docs',
    description: 'Generate API documentation structure',
    runAfter: ['cpp-extractor'],
    runBefore: ['rendering-docs'],

    /**
     * @description Process documents to create API documentation structure
     * @param {Array} docs - Array of document objects
     * @returns {Array} Processed documents with API structure
     */
    process: function(docs) {
      const apiDocs = {
        modules: {},
        classes: {},
        functions: {},
        guides: {}
      };

      docs.forEach(doc => {
        if (doc.docType === 'guide') {
          apiDocs.guides[doc.id] = doc;
        } else if (doc.classes) {
          doc.classes.forEach(cls => {
            apiDocs.classes[cls.name] = cls;
          });
        } else if (doc.functions) {
          doc.functions.forEach(func => {
            apiDocs.functions[func.name] = func;
          });
        }
      });

      // Create index document
      docs.push({
        docType: 'api-index',
        id: 'api-index',
        title: 'UMICP API Reference',
        template: 'api-index.template.html',
        outputPath: 'index.html',
        apiStructure: apiDocs
      });

      return docs;
    }
  };
})

// Configure output file processor
.config(function(computePathsProcessor) {

  /**
   * @description Path templates for different document types
   */
  computePathsProcessor.pathTemplates = [
    {
      docTypes: ['api-index'],
      pathTemplate: '${id}',
      outputPathTemplate: '${path}.html'
    },
    {
      docTypes: ['class'],
      pathTemplate: 'classes/${name}',
      outputPathTemplate: '${path}.html'
    },
    {
      docTypes: ['function'],
      pathTemplate: 'functions/${name}',
      outputPathTemplate: '${path}.html'
    },
    {
      docTypes: ['guide'],
      pathTemplate: 'guides/${id}',
      outputPathTemplate: '${path}.html'
    },
    {
      docTypes: ['module'],
      pathTemplate: 'modules/${id}',
      outputPathTemplate: '${path}.html'
    }
  ];
});
