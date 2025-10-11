"""Run all UMICP performance benchmarks."""

import sys
import platform


def print_system_info():
    """Print system information."""
    print("=" * 70)
    print("UMICP Python Bindings - Performance Benchmarks")
    print("=" * 70)
    print(f"Python version: {sys.version}")
    print(f"Platform: {platform.platform()}")
    print(f"Processor: {platform.processor()}")
    print("=" * 70)


def main():
    """Run all benchmarks."""
    print_system_info()

    # Import and run benchmarks
    try:
        print("\n\nRunning Matrix Benchmarks...")
        import benchmark_matrix
        benchmark_matrix.main()
    except Exception as e:
        print(f"Error running matrix benchmarks: {e}")

    try:
        print("\n\nRunning Envelope Benchmarks...")
        import benchmark_envelope
        benchmark_envelope.main()
    except Exception as e:
        print(f"Error running envelope benchmarks: {e}")

    print("\n\n" + "=" * 70)
    print("All benchmarks completed!")
    print("=" * 70)


if __name__ == "__main__":
    main()

