# Segmentation Fault Fix - UMICP TypeScript Binding

## Problem

The TypeScript binding tests were experiencing segmentation faults when running with Vitest:

```bash
$ npm test
Segmentation fault (core dumped)
```

## Root Cause

The **native C++ addon is not thread-safe**. When Vitest runs tests in parallel using multiple threads, concurrent access to the native module causes memory corruption and segmentation faults.

### Why This Happens

1. **Native modules and threading**: Node.js native addons (using N-API) are designed to work in a single-threaded context
2. **Vitest's default behavior**: By default, Vitest uses worker threads to run tests in parallel
3. **Concurrent access**: Multiple threads trying to access the same native module simultaneously leads to:
   - Race conditions
   - Memory corruption
   - Segmentation faults

## Solution

Configure Vitest to run tests **sequentially in a single process** instead of using parallel threads/forks:

### vitest.config.ts Configuration

```typescript
export default defineConfig({
  test: {
    // ... other config ...
    
    // Disable isolation to prevent forking
    isolate: false,
    
    // Use forks pool with single fork
    pool: 'forks',
    poolOptions: {
      forks: {
        singleFork: true,
      }
    },
    
    // Disable file parallelization
    fileParallelism: false,
    
    // Run tests sequentially
    sequence: {
      concurrent: false,
      shuffle: false,
    }
  }
});
```

## Key Configuration Changes

1. **`isolate: false`**: Prevents each test file from running in a separate environment
2. **`pool: 'forks'`**: Uses process forks instead of threads (safer for native modules)
3. **`singleFork: true`**: Ensures all tests run in a single forked process
4. **`fileParallelism: false`**: Disables parallel execution of test files
5. **`sequence.concurrent: false`**: Runs tests sequentially, not concurrently

## Verification

After applying these changes:

```bash
$ npm test

 ✓ test/envelope.test.ts  (22 tests) 394ms
 ✓ test/matrix.test.ts   (28 tests) 6323ms

 Test Files  2 passed (2)
      Tests  50 passed (50)
```

✅ No more segmentation faults!

## Trade-offs

### Advantages
- ✅ No segmentation faults
- ✅ Reliable test execution
- ✅ Predictable behavior

### Disadvantages
- ⚠️ Slower test execution (tests run sequentially)
- ⚠️ Cannot leverage multi-core parallelism
- ⚠️ Longer CI/CD pipeline times

## Alternative Solutions (Not Recommended)

1. **Make the native module thread-safe**: Would require significant refactoring of C++ code with mutexes and proper synchronization
2. **Use Jest instead of Vitest**: Jest's default configuration is more compatible with native modules
3. **Run tests in Docker**: Isolates each test run but adds overhead

## Best Practices for Native Module Testing

1. **Always test with actual test runner**: Simple node scripts may work, but test runners expose threading issues
2. **Document threading limitations**: Make it clear in README that the module is not thread-safe
3. **Use process isolation**: Prefer process forks over threads when dealing with native code
4. **Consider refactoring**: If performance is critical, invest in making the native module thread-safe

## Related Issues

- Node.js native addons: https://nodejs.org/api/addons.html
- N-API thread safety: https://nodejs.org/api/n-api.html#worker-support
- Vitest configuration: https://vitest.dev/config/

## Status

✅ **FIXED** - Segmentation faults resolved with proper Vitest configuration


