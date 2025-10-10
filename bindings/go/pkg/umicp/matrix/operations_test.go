package matrix

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestDotProduct(t *testing.T) {
	tests := []struct {
		name    string
		a       []float32
		b       []float32
		want    float32
		wantErr bool
	}{
		{
			name: "simple dot product",
			a:    []float32{1, 2, 3},
			b:    []float32{4, 5, 6},
			want: 32, // 1*4 + 2*5 + 3*6 = 32
		},
		{
			name:    "mismatched lengths",
			a:       []float32{1, 2},
			b:       []float32{1, 2, 3},
			wantErr: true,
		},
		{
			name: "zero vectors",
			a:    []float32{0, 0, 0},
			b:    []float32{0, 0, 0},
			want: 0,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := DotProduct(tt.a, tt.b)
			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.InDelta(t, tt.want, got, 0.0001)
			}
		})
	}
}

func TestMagnitude(t *testing.T) {
	tests := []struct {
		name string
		v    []float32
		want float32
	}{
		{
			name: "unit vector",
			v:    []float32{1, 0, 0},
			want: 1,
		},
		{
			name: "3-4-5 triangle",
			v:    []float32{3, 4},
			want: 5,
		},
		{
			name: "zero vector",
			v:    []float32{0, 0, 0},
			want: 0,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := Magnitude(tt.v)
			assert.InDelta(t, tt.want, got, 0.0001)
		})
	}
}

func TestCosineSimilarity(t *testing.T) {
	tests := []struct {
		name    string
		a       []float32
		b       []float32
		want    float32
		wantErr bool
	}{
		{
			name: "identical vectors",
			a:    []float32{1, 2, 3},
			b:    []float32{1, 2, 3},
			want: 1.0,
		},
		{
			name: "orthogonal vectors",
			a:    []float32{1, 0},
			b:    []float32{0, 1},
			want: 0.0,
		},
		{
			name: "opposite vectors",
			a:    []float32{1, 2, 3},
			b:    []float32{-1, -2, -3},
			want: -1.0,
		},
		{
			name: "zero vector",
			a:    []float32{0, 0, 0},
			b:    []float32{1, 2, 3},
			want: 0.0,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := CosineSimilarity(tt.a, tt.b)
			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.InDelta(t, tt.want, got, 0.0001)
			}
		})
	}
}

func TestEuclideanDistance(t *testing.T) {
	tests := []struct {
		name string
		a    []float32
		b    []float32
		want float32
	}{
		{
			name: "identical points",
			a:    []float32{1, 2, 3},
			b:    []float32{1, 2, 3},
			want: 0,
		},
		{
			name: "3-4-5 triangle",
			a:    []float32{0, 0},
			b:    []float32{3, 4},
			want: 5,
		},
		{
			name: "unit distance",
			a:    []float32{0, 0},
			b:    []float32{1, 0},
			want: 1,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := EuclideanDistance(tt.a, tt.b)
			assert.NoError(t, err)
			assert.InDelta(t, tt.want, got, 0.0001)
		})
	}
}

func TestVectorAdd(t *testing.T) {
	a := []float32{1, 2, 3}
	b := []float32{4, 5, 6}
	result := make([]float32, 3)
	want := []float32{5, 7, 9}

	err := VectorAdd(a, b, result)
	assert.NoError(t, err)
	assert.Equal(t, want, result)
}

func TestVectorSubtract(t *testing.T) {
	a := []float32{5, 7, 9}
	b := []float32{1, 2, 3}
	result := make([]float32, 3)
	want := []float32{4, 5, 6}

	err := VectorSubtract(a, b, result)
	assert.NoError(t, err)
	assert.Equal(t, want, result)
}

func TestScalarMultiply(t *testing.T) {
	v := []float32{1, 2, 3}
	scalar := float32(2)
	result := make([]float32, 3)
	want := []float32{2, 4, 6}

	err := ScalarMultiply(v, scalar, result)
	assert.NoError(t, err)
	assert.Equal(t, want, result)
}

func TestMatrixMultiply(t *testing.T) {
	// 2x2 matrix multiplication
	a := []float32{1, 2, 3, 4} // [[1,2], [3,4]]
	b := []float32{5, 6, 7, 8} // [[5,6], [7,8]]
	result := make([]float32, 4)
	want := []float32{19, 22, 43, 50} // [[19,22], [43,50]]

	err := MatrixMultiply(a, b, result, 2, 2, 2)
	assert.NoError(t, err)

	for i := range want {
		assert.InDelta(t, want[i], result[i], 0.0001)
	}
}

func TestNormalize(t *testing.T) {
	v := []float32{3, 4}
	result := make([]float32, 2)
	want := []float32{0.6, 0.8} // 3/5, 4/5

	err := Normalize(v, result)
	assert.NoError(t, err)

	for i := range want {
		assert.InDelta(t, want[i], result[i], 0.0001)
	}

	// Check that normalized vector has magnitude 1
	mag := Magnitude(result)
	assert.InDelta(t, 1.0, mag, 0.0001)
}

func TestManhattanDistance(t *testing.T) {
	a := []float32{1, 2, 3}
	b := []float32{4, 6, 8}
	want := float32(10) // |1-4| + |2-6| + |3-8| = 3 + 4 + 5 = 12? No: 3 + 4 + 5 = 12, but wait |3-8| = 5, so 3+4+5 = 12 actually

	got, err := ManhattanDistance(a, b)
	assert.NoError(t, err)

	// Calculate manually: |1-4| + |2-6| + |3-8| = 3 + 4 + 5 = 12
	assert.InDelta(t, 12.0, got, 0.0001)
}

func BenchmarkDotProduct(b *testing.B) {
	v1 := make([]float32, 512)
	v2 := make([]float32, 512)
	for i := range v1 {
		v1[i] = float32(i)
		v2[i] = float32(i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = DotProduct(v1, v2)
	}
}

func BenchmarkCosineSimilarity(b *testing.B) {
	v1 := make([]float32, 512)
	v2 := make([]float32, 512)
	for i := range v1 {
		v1[i] = float32(i)
		v2[i] = float32(i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = CosineSimilarity(v1, v2)
	}
}

func BenchmarkMatrixMultiply(b *testing.B) {
	size := 10
	a := make([]float32, size*size)
	bMat := make([]float32, size*size)
	result := make([]float32, size*size)

	for i := range a {
		a[i] = float32(i)
		bMat[i] = float32(i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = MatrixMultiply(a, bMat, result, size, size, size)
	}
}
