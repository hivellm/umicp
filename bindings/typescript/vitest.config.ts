import { defineConfig } from 'vitest/config';
import { resolve } from 'path';

// IMPORTANT: Native modules are not thread-safe!
// Using pool: 'forks' to avoid segmentation faults when running tests
// Each test file runs in a separate process (fork) instead of a thread
export default defineConfig({
  test: {
    globals: true,
    environment: 'node',
    setupFiles: ['./test/setup.ts'],
    include: ['test/**/*.test.ts'],
    exclude: ['node_modules', 'dist', 'build'],
    coverage: {
      provider: 'v8',
      reporter: ['text', 'lcov', 'html', 'json'],
      include: ['src/**/*.ts'],
      exclude: [
        'src/**/*.d.ts',
        'src/**/websocket_wrap.*',
        'src/**/http2_wrap.*',
        'examples/**/*.ts',
        'examples/**/benchmark.js'
      ],
      all: true,
      thresholds: {
        branches: 70,
        functions: 80,
        lines: 80,
        statements: 80
      }
    },
    testTimeout: 60000,
    hookTimeout: 60000,
    teardownTimeout: 10000,
    isolate: false,
    // Disable worker threads/forks to prevent segfaults with native modules
    // Run all tests sequentially in the main process
    pool: 'forks',
    poolOptions: {
      forks: {
        singleFork: true,
      }
    },
    // Disable file parallelization
    fileParallelism: false,
    sequence: {
      concurrent: false,
      shuffle: false,
    }
  },
  resolve: {
    alias: {
      '@': resolve(__dirname, './src'),
      'examples': resolve(__dirname, './examples')
    }
  }
});

