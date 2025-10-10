package benchmark

import (
	"testing"

	"github.com/hivellm/umicp-go/pkg/umicp"
	"github.com/hivellm/umicp-go/pkg/umicp/matrix"
)

// Envelope Benchmarks
func BenchmarkEnvelope_CreateSimple(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_, _ = umicp.NewEnvelope().
			From("a").
			To("b").
			Build()
	}
}

func BenchmarkEnvelope_CreateComplex(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_, _ = umicp.NewEnvelope().
			From("sender").
			To("receiver").
			Operation(umicp.OperationData).
			Capability("k1", "v1").
			Capability("k2", "v2").
			Capability("k3", "v3").
			Payload([]byte("test"), umicp.PayloadText).
			Build()
	}
}

func BenchmarkEnvelope_SerializeSmall(b *testing.B) {
	env, _ := umicp.NewEnvelope().From("a").To("b").Build()
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = env.Serialize()
	}
}

func BenchmarkEnvelope_SerializeLarge(b *testing.B) {
	payload := make([]byte, 10*1024) // 10KB
	env, _ := umicp.NewEnvelope().
		From("a").
		To("b").
		Payload(payload, umicp.PayloadBinary).
		Build()

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = env.Serialize()
	}
}

// Matrix Benchmarks
func BenchmarkMatrix_DotProduct_Small(b *testing.B) {
	v1 := make([]float32, 128)
	v2 := make([]float32, 128)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = matrix.DotProduct(v1, v2)
	}
}

func BenchmarkMatrix_DotProduct_Large(b *testing.B) {
	v1 := make([]float32, 1536) // OpenAI embedding size
	v2 := make([]float32, 1536)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = matrix.DotProduct(v1, v2)
	}
}

func BenchmarkMatrix_CosineSimilarity(b *testing.B) {
	v1 := make([]float32, 512)
	v2 := make([]float32, 512)
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = matrix.CosineSimilarity(v1, v2)
	}
}

func BenchmarkMatrix_MatrixMultiply_10x10(b *testing.B) {
	size := 10
	a := make([]float32, size*size)
	bMat := make([]float32, size*size)
	result := make([]float32, size*size)

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = matrix.MatrixMultiply(a, bMat, result, size, size, size)
	}
}

func BenchmarkMatrix_Normalize(b *testing.B) {
	v := make([]float32, 512)
	result := make([]float32, 512)
	for i := range v {
		v[i] = float32(i)
	}

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = matrix.Normalize(v, result)
	}
}
