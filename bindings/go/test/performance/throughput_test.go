package performance

import (
	"testing"

	"github.com/hivellm/umicp-go/pkg/umicp"
)

func BenchmarkEnvelope_Create(b *testing.B) {
	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		_, _ = umicp.NewEnvelope().
			From("sender").
			To("receiver").
			Operation(umicp.OperationData).
			Build()
	}
}

func BenchmarkEnvelope_Serialize(b *testing.B) {
	env, _ := umicp.NewEnvelope().
		From("sender").
		To("receiver").
		Operation(umicp.OperationData).
		Capability("key1", "value1").
		Capability("key2", "value2").
		Build()

	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		_, _ = env.Serialize()
	}
}

func BenchmarkEnvelope_Deserialize(b *testing.B) {
	env, _ := umicp.NewEnvelope().
		From("sender").
		To("receiver").
		Operation(umicp.OperationData).
		Build()

	data, _ := env.Serialize()

	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		_, _ = umicp.DeserializeEnvelope(data)
	}
}

func BenchmarkEnvelope_Hash(b *testing.B) {
	env, _ := umicp.NewEnvelope().
		From("sender").
		To("receiver").
		Build()

	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		_ = env.Hash()
	}
}

func BenchmarkEnvelope_Clone(b *testing.B) {
	env, _ := umicp.NewEnvelope().
		From("sender").
		To("receiver").
		Operation(umicp.OperationData).
		Capability("key", "value").
		Payload([]byte("test data"), umicp.PayloadText).
		Build()

	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		_ = env.Clone()
	}
}

func BenchmarkEnvelope_WithPayload(b *testing.B) {
	payload := make([]byte, 1024) // 1KB payload

	b.ReportAllocs()
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		_, _ = umicp.NewEnvelope().
			From("sender").
			To("receiver").
			Operation(umicp.OperationData).
			Payload(payload, umicp.PayloadBinary).
			Build()
	}
}
