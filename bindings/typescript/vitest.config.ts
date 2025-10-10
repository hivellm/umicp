import { defineConfig } from 'vitest/config';
import { resolve } from 'path';

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
    isolate: true,
    threads: true,
    maxThreads: 4,
    minThreads: 1
  },
  resolve: {
    alias: {
      '@': resolve(__dirname, './src'),
      'examples': resolve(__dirname, './examples')
    }
  }
});

