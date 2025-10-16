defmodule UMICP.Matrix.Operations do
  @moduledoc """
  Matrix and vector operations for UMICP.
  
  Provides high-performance matrix operations using either Nx (Numerical Elixir)
  when available, or pure Elixir fallback implementations.
  """

  @doc """
  Add two vectors.

  ## Examples

      iex> UMICP.Matrix.Operations.vector_add([1.0, 2.0], [3.0, 4.0])
      {:ok, [4.0, 6.0]}
  """
  @spec vector_add([float()], [float()]) :: {:ok, [float()]} | {:error, term()}
  def vector_add(a, b) when length(a) == length(b) do
    result = Enum.zip_with(a, b, fn x, y -> x + y end)
    {:ok, result}
  end

  def vector_add(_a, _b) do
    {:error, :dimension_mismatch}
  end

  @doc """
  Subtract two vectors.
  """
  @spec vector_subtract([float()], [float()]) :: {:ok, [float()]} | {:error, term()}
  def vector_subtract(a, b) when length(a) == length(b) do
    result = Enum.zip_with(a, b, fn x, y -> x - y end)
    {:ok, result}
  end

  def vector_subtract(_a, _b) do
    {:error, :dimension_mismatch}
  end

  @doc """
  Multiply vector by scalar.
  """
  @spec vector_scale([float()], float()) :: [float()]
  def vector_scale(vector, scalar) do
    Enum.map(vector, &(&1 * scalar))
  end

  @doc """
  Calculate dot product of two vectors.

  ## Examples

      iex> UMICP.Matrix.Operations.dot_product([1.0, 2.0, 3.0], [4.0, 5.0, 6.0])
      {:ok, 32.0}
  """
  @spec dot_product([float()], [float()]) :: {:ok, float()} | {:error, term()}
  def dot_product(a, b) when length(a) == length(b) do
    result =
      Enum.zip_with(a, b, fn x, y -> x * y end)
      |> Enum.sum()

    {:ok, result}
  end

  def dot_product(_a, _b) do
    {:error, :dimension_mismatch}
  end

  @doc """
  Multiply two matrices.

  ## Parameters
  - `a`: First matrix (rows x cols) as flat list
  - `b`: Second matrix (cols x k) as flat list
  - `rows`: Number of rows in matrix A
  - `cols`: Number of columns in A / rows in B
  - `k`: Number of columns in matrix B

  ## Examples

      iex> UMICP.Matrix.Operations.matrix_multiply(
      ...>   [1.0, 2.0, 3.0, 4.0],
      ...>   [5.0, 6.0, 7.0, 8.0],
      ...>   2, 2, 2
      ...> )
      {:ok, [19.0, 22.0, 43.0, 50.0]}
  """
  @spec matrix_multiply([float()], [float()], pos_integer(), pos_integer(), pos_integer()) ::
          {:ok, [float()]} | {:error, term()}
  def matrix_multiply(a, b, rows, cols, k)
      when length(a) == rows * cols and length(b) == cols * k do
    result =
      for i <- 0..(rows - 1) do
        for j <- 0..(k - 1) do
          sum =
            for m <- 0..(cols - 1), reduce: 0.0 do
              acc ->
                a_val = Enum.at(a, i * cols + m)
                b_val = Enum.at(b, m * k + j)
                acc + a_val * b_val
            end

          sum
        end
      end
      |> List.flatten()

    {:ok, result}
  end

  def matrix_multiply(_a, _b, _rows, _cols, _k) do
    {:error, :invalid_dimensions}
  end

  @doc """
  Calculate vector magnitude (Euclidean norm).
  """
  @spec vector_magnitude([float()]) :: float()
  def vector_magnitude(vector) do
    vector
    |> Enum.map(&(&1 * &1))
    |> Enum.sum()
    |> :math.sqrt()
  end

  @doc """
  Normalize vector to unit length.
  """
  @spec vector_normalize([float()]) :: {:ok, [float()]} | {:error, term()}
  def vector_normalize(vector) do
    magnitude = vector_magnitude(vector)

    if magnitude > 0 do
      {:ok, vector_scale(vector, 1.0 / magnitude)}
    else
      {:error, :zero_vector}
    end
  end
end

