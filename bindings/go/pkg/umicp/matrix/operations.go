package matrix

import (
	"fmt"
	"math"
)

// DotProduct computes the dot product of two vectors
func DotProduct(a, b []float32) (float32, error) {
	if len(a) != len(b) {
		return 0, fmt.Errorf("vectors must have the same length: got %d and %d", len(a), len(b))
	}

	var sum float32
	for i := range a {
		sum += a[i] * b[i]
	}

	return sum, nil
}

// Magnitude computes the Euclidean norm (magnitude) of a vector
func Magnitude(v []float32) float32 {
	var sum float32
	for _, val := range v {
		sum += val * val
	}
	return float32(math.Sqrt(float64(sum)))
}

// CosineSimilarity computes the cosine similarity between two vectors
func CosineSimilarity(a, b []float32) (float32, error) {
	if len(a) != len(b) {
		return 0, fmt.Errorf("vectors must have the same length: got %d and %d", len(a), len(b))
	}

	dot, err := DotProduct(a, b)
	if err != nil {
		return 0, err
	}

	magA := Magnitude(a)
	magB := Magnitude(b)

	if magA == 0 || magB == 0 {
		return 0, nil
	}

	return dot / (magA * magB), nil
}

// EuclideanDistance computes the Euclidean distance between two vectors
func EuclideanDistance(a, b []float32) (float32, error) {
	if len(a) != len(b) {
		return 0, fmt.Errorf("vectors must have the same length: got %d and %d", len(a), len(b))
	}

	var sum float32
	for i := range a {
		diff := a[i] - b[i]
		sum += diff * diff
	}

	return float32(math.Sqrt(float64(sum))), nil
}

// VectorAdd performs element-wise addition of two vectors
func VectorAdd(a, b, result []float32) error {
	if len(a) != len(b) || len(a) != len(result) {
		return fmt.Errorf("vectors must have the same length")
	}

	for i := range a {
		result[i] = a[i] + b[i]
	}

	return nil
}

// VectorSubtract performs element-wise subtraction of two vectors
func VectorSubtract(a, b, result []float32) error {
	if len(a) != len(b) || len(a) != len(result) {
		return fmt.Errorf("vectors must have the same length")
	}

	for i := range a {
		result[i] = a[i] - b[i]
	}

	return nil
}

// ScalarMultiply multiplies a vector by a scalar
func ScalarMultiply(v []float32, scalar float32, result []float32) error {
	if len(v) != len(result) {
		return fmt.Errorf("vectors must have the same length")
	}

	for i := range v {
		result[i] = v[i] * scalar
	}

	return nil
}

// MatrixMultiply multiplies two matrices
// A is m x n, B is n x p, result is m x p
func MatrixMultiply(a, b, result []float32, m, n, p int) error {
	if len(a) != m*n {
		return fmt.Errorf("matrix A size mismatch: expected %d, got %d", m*n, len(a))
	}
	if len(b) != n*p {
		return fmt.Errorf("matrix B size mismatch: expected %d, got %d", n*p, len(b))
	}
	if len(result) != m*p {
		return fmt.Errorf("result matrix size mismatch: expected %d, got %d", m*p, len(result))
	}

	for i := 0; i < m; i++ {
		for j := 0; j < p; j++ {
			var sum float32
			for k := 0; k < n; k++ {
				sum += a[i*n+k] * b[k*p+j]
			}
			result[i*p+j] = sum
		}
	}

	return nil
}

// Normalize normalizes a vector to unit length
func Normalize(v, result []float32) error {
	if len(v) != len(result) {
		return fmt.Errorf("vectors must have the same length")
	}

	mag := Magnitude(v)
	if mag == 0 {
		return fmt.Errorf("cannot normalize zero vector")
	}

	for i := range v {
		result[i] = v[i] / mag
	}

	return nil
}

// ManhattanDistance computes the Manhattan (L1) distance between two vectors
func ManhattanDistance(a, b []float32) (float32, error) {
	if len(a) != len(b) {
		return 0, fmt.Errorf("vectors must have the same length: got %d and %d", len(a), len(b))
	}

	var sum float32
	for i := range a {
		diff := a[i] - b[i]
		if diff < 0 {
			diff = -diff
		}
		sum += diff
	}

	return sum, nil
}
