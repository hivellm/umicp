# UMICP Python Performance Benchmarks

Performance benchmarks for UMICP Python bindings.

## Running Benchmarks

### Run All Benchmarks

```bash
cd umicp/bindings/python/benchmarks
python run_all_benchmarks.py
```

### Run Individual Benchmarks

```bash
# Matrix operations
python benchmark_matrix.py

# Envelope operations
python benchmark_envelope.py
```

## Benchmark Results

Benchmarks measure:
- Operations per second (ops/sec)
- Milliseconds per operation (ms/op)

### Matrix Operations
- **Dot Product**: Vector dot product with various sizes
- **Cosine Similarity**: Cosine similarity calculation
- **Vector Addition**: Element-wise vector addition
- **Matrix Multiplication**: Matrix multiplication with various dimensions

### Envelope Operations
- **Creation**: Envelope object creation
- **Serialization**: JSON serialization
- **Deserialization**: JSON deserialization
- **Large Envelopes**: Operations with many capabilities

## Requirements

```bash
pip install numpy
```

## Expected Performance

On a modern CPU (as reference):
- Dot product (1000 elements): ~100,000 ops/sec
- Envelope serialization: ~50,000 ops/sec
- Envelope deserialization: ~30,000 ops/sec

Actual performance depends on hardware and system load.

