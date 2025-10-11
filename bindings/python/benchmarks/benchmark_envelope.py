"""Performance benchmarks for UMICP Envelope operations."""

import time
from umicp.envelope import Envelope
from umicp.types import OperationType


def benchmark_envelope_creation(iterations=10000):
    """Benchmark envelope creation."""
    print("\n=== Envelope Creation Benchmark ===")

    # Warmup
    for _ in range(100):
        env = Envelope(
            from_id="client-1",
            to_id="server-1",
            operation=OperationType.DATA,
            message_id="msg-123"
        )

    # Benchmark
    start = time.perf_counter()
    for i in range(iterations):
        env = Envelope(
            from_id="client-1",
            to_id="server-1",
            operation=OperationType.DATA,
            message_id=f"msg-{i}"
        )
    end = time.perf_counter()

    elapsed = end - start
    ops_per_sec = iterations / elapsed

    print(f"Created {iterations} envelopes in {elapsed:.3f}s")
    print(f"Rate: {ops_per_sec:,.0f} ops/sec, {elapsed/iterations*1000:.3f} ms/op")


def benchmark_envelope_serialization(iterations=10000):
    """Benchmark envelope serialization to JSON."""
    print("\n=== Envelope Serialization Benchmark ===")

    env = Envelope(
        from_id="client-1",
        to_id="server-1",
        operation=OperationType.DATA,
        message_id="msg-123"
    )
    env.add_capability("key1", "value1")
    env.add_capability("key2", "value2")

    # Warmup
    for _ in range(100):
        json_str = env.to_json()

    # Benchmark
    start = time.perf_counter()
    for _ in range(iterations):
        json_str = env.to_json()
    end = time.perf_counter()

    elapsed = end - start
    ops_per_sec = iterations / elapsed

    print(f"Serialized {iterations} envelopes in {elapsed:.3f}s")
    print(f"Rate: {ops_per_sec:,.0f} ops/sec, {elapsed/iterations*1000:.3f} ms/op")
    print(f"JSON size: {len(json_str)} bytes")


def benchmark_envelope_deserialization(iterations=10000):
    """Benchmark envelope deserialization from JSON."""
    print("\n=== Envelope Deserialization Benchmark ===")

    env = Envelope(
        from_id="client-1",
        to_id="server-1",
        operation=OperationType.DATA,
        message_id="msg-123"
    )
    env.add_capability("key1", "value1")
    env.add_capability("key2", "value2")

    json_str = env.to_json()

    # Warmup
    for _ in range(100):
        parsed = Envelope.from_json(json_str)

    # Benchmark
    start = time.perf_counter()
    for _ in range(iterations):
        parsed = Envelope.from_json(json_str)
    end = time.perf_counter()

    elapsed = end - start
    ops_per_sec = iterations / elapsed

    print(f"Deserialized {iterations} envelopes in {elapsed:.3f}s")
    print(f"Rate: {ops_per_sec:,.0f} ops/sec, {elapsed/iterations*1000:.3f} ms/op")


def benchmark_envelope_with_large_capabilities(iterations=1000):
    """Benchmark envelope with many capabilities."""
    print("\n=== Large Envelope Benchmark (100 capabilities) ===")

    # Create envelope with 100 capabilities
    env = Envelope(
        from_id="client-1",
        to_id="server-1",
        operation=OperationType.DATA,
        message_id="msg-123"
    )
    for i in range(100):
        env.add_capability(f"key{i}", f"value{i}")

    # Serialization benchmark
    start = time.perf_counter()
    for _ in range(iterations):
        json_str = env.to_json()
    end = time.perf_counter()

    elapsed = end - start
    ops_per_sec = iterations / elapsed

    print(f"Serialization: {ops_per_sec:,.0f} ops/sec, {elapsed/iterations*1000:.3f} ms/op")
    print(f"JSON size: {len(json_str)} bytes")

    # Deserialization benchmark
    json_str = env.to_json()
    start = time.perf_counter()
    for _ in range(iterations):
        parsed = Envelope.from_json(json_str)
    end = time.perf_counter()

    elapsed = end - start
    ops_per_sec = iterations / elapsed

    print(f"Deserialization: {ops_per_sec:,.0f} ops/sec, {elapsed/iterations*1000:.3f} ms/op")


def main():
    """Run all benchmarks."""
    print("=" * 60)
    print("UMICP Envelope Performance Benchmarks")
    print("=" * 60)

    benchmark_envelope_creation()
    benchmark_envelope_serialization()
    benchmark_envelope_deserialization()
    benchmark_envelope_with_large_capabilities()

    print("\n" + "=" * 60)


if __name__ == "__main__":
    main()

