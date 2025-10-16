defmodule UMICP.Matrix.OperationsTest do
  use ExUnit.Case, async: true

  alias UMICP.Matrix.Operations

  describe "vector_add/2" do
    test "adds two vectors" do
      assert {:ok, [5.0, 7.0, 9.0]} = Operations.vector_add([1.0, 2.0, 3.0], [4.0, 5.0, 6.0])
    end

    test "returns error for mismatched dimensions" do
      assert {:error, :dimension_mismatch} = Operations.vector_add([1.0, 2.0], [1.0, 2.0, 3.0])
    end
  end

  describe "vector_subtract/2" do
    test "subtracts two vectors" do
      assert {:ok, [3.0, 3.0, 3.0]} = Operations.vector_subtract([4.0, 5.0, 6.0], [1.0, 2.0, 3.0])
    end

    test "returns error for mismatched dimensions" do
      assert {:error, :dimension_mismatch} = Operations.vector_subtract([1.0], [1.0, 2.0])
    end
  end

  describe "vector_scale/2" do
    test "scales vector by scalar" do
      result = Operations.vector_scale([1.0, 2.0, 3.0], 2.0)
      assert result == [2.0, 4.0, 6.0]
    end

    test "scales by zero" do
      result = Operations.vector_scale([1.0, 2.0, 3.0], 0.0)
      assert result == [0.0, 0.0, 0.0]
    end
  end

  describe "dot_product/2" do
    test "calculates dot product" do
      # 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
      assert {:ok, 32.0} = Operations.dot_product([1.0, 2.0, 3.0], [4.0, 5.0, 6.0])
    end

    test "returns error for mismatched dimensions" do
      assert {:error, :dimension_mismatch} = Operations.dot_product([1.0, 2.0], [1.0])
    end
  end

  describe "matrix_multiply/5" do
    test "multiplies 2x2 matrices" do
      # [[1,2], [3,4]] * [[5,6], [7,8]]
      # = [[19,22], [43,50]]
      a = [1.0, 2.0, 3.0, 4.0]
      b = [5.0, 6.0, 7.0, 8.0]

      assert {:ok, result} = Operations.matrix_multiply(a, b, 2, 2, 2)
      assert_in_delta Enum.at(result, 0), 19.0, 0.001
      assert_in_delta Enum.at(result, 1), 22.0, 0.001
      assert_in_delta Enum.at(result, 2), 43.0, 0.001
      assert_in_delta Enum.at(result, 3), 50.0, 0.001
    end

    test "returns error for invalid dimensions" do
      assert {:error, :invalid_dimensions} = Operations.matrix_multiply([1.0, 2.0], [3.0], 2, 1, 1)
    end
  end

  describe "vector_magnitude/1" do
    test "calculates magnitude" do
      # sqrt(3^2 + 4^2) = sqrt(9 + 16) = 5
      magnitude = Operations.vector_magnitude([3.0, 4.0])
      assert_in_delta magnitude, 5.0, 0.001
    end

    test "returns zero for zero vector" do
      magnitude = Operations.vector_magnitude([0.0, 0.0, 0.0])
      assert_in_delta magnitude, 0.0, 0.001
    end
  end

  describe "vector_normalize/1" do
    test "normalizes vector" do
      assert {:ok, normalized} = Operations.vector_normalize([3.0, 4.0])
      assert_in_delta Enum.at(normalized, 0), 0.6, 0.001
      assert_in_delta Enum.at(normalized, 1), 0.8, 0.001

      # Verify unit length
      magnitude = Operations.vector_magnitude(normalized)
      assert_in_delta magnitude, 1.0, 0.001
    end

    test "returns error for zero vector" do
      assert {:error, :zero_vector} = Operations.vector_normalize([0.0, 0.0])
    end
  end
end

