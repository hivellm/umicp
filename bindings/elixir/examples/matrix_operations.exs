#!/usr/bin/env elixir

# Matrix Operations Example for UMICP Elixir SDK

Code.prepend_path("_build/dev/lib/umicp/ebin")

defmodule MatrixOperationsExample do
  alias UMICP.Matrix.Operations

  def run do
    IO.puts("\n=== Matrix Operations Example ===\n")

    # Vector addition
    IO.puts("Vector Addition:")
    v1 = [1.0, 2.0, 3.0, 4.0]
    v2 = [5.0, 6.0, 7.0, 8.0]
    {:ok, sum} = Operations.vector_add(v1, v2)
    IO.puts("  #{inspect(v1)} + #{inspect(v2)} = #{inspect(sum)}")

    # Vector subtraction
    IO.puts("\nVector Subtraction:")
    {:ok, diff} = Operations.vector_subtract(v2, v1)
    IO.puts("  #{inspect(v2)} - #{inspect(v1)} = #{inspect(diff)}")

    # Scalar multiplication
    IO.puts("\nScalar Multiplication:")
    scaled = Operations.vector_scale(v1, 2.0)
    IO.puts("  #{inspect(v1)} * 2.0 = #{inspect(scaled)}")

    # Dot product
    IO.puts("\nDot Product:")
    {:ok, dot} = Operations.dot_product(v1, v2)
    IO.puts("  #{inspect(v1)} · #{inspect(v2)} = #{dot}")

    # Vector magnitude
    IO.puts("\nVector Magnitude:")
    test_vector = [3.0, 4.0]
    magnitude = Operations.vector_magnitude(test_vector)
    IO.puts("  ||#{inspect(test_vector)}|| = #{magnitude}")

    # Vector normalization
    IO.puts("\nVector Normalization:")
    {:ok, normalized} = Operations.vector_normalize(test_vector)
    IO.puts("  normalize(#{inspect(test_vector)}) = #{inspect(normalized)}")
    norm_magnitude = Operations.vector_magnitude(normalized)
    IO.puts("  magnitude of normalized = #{norm_magnitude}")

    # Matrix multiplication
    IO.puts("\nMatrix Multiplication (2x2):")
    m1 = [1.0, 2.0, 3.0, 4.0]
    m2 = [5.0, 6.0, 7.0, 8.0]
    IO.puts("  [[1, 2],     [[5, 6],")
    IO.puts("   [3, 4]]  *   [7, 8]]")

    {:ok, result} = Operations.matrix_multiply(m1, m2, 2, 2, 2)
    [r0, r1, r2, r3] = result
    IO.puts("  = [[#{r0}, #{r1}],")
    IO.puts("     [#{r2}, #{r3}]]")

    IO.puts("\n✅ Matrix operations example completed!")
  end
end

MatrixOperationsExample.run()

