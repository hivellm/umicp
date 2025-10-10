package umicp

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNewEnvelope(t *testing.T) {
	builder := NewEnvelope()
	require.NotNil(t, builder)
	require.NotNil(t, builder.envelope)
	assert.NotEmpty(t, builder.envelope.MessageID)
	assert.NotZero(t, builder.envelope.Timestamp)
	assert.NotNil(t, builder.envelope.Capabilities)
}

func TestEnvelopeBuilder_Build(t *testing.T) {
	tests := []struct {
		name    string
		setup   func() *EnvelopeBuilder
		wantErr bool
		errType error
	}{
		{
			name: "valid envelope",
			setup: func() *EnvelopeBuilder {
				return NewEnvelope().
					From("sender").
					To("receiver").
					Operation(OperationData)
			},
			wantErr: false,
		},
		{
			name: "missing from",
			setup: func() *EnvelopeBuilder {
				return NewEnvelope().
					To("receiver").
					Operation(OperationData)
			},
			wantErr: true,
			errType: ErrMissingFrom,
		},
		{
			name: "missing to",
			setup: func() *EnvelopeBuilder {
				return NewEnvelope().
					From("sender").
					Operation(OperationData)
			},
			wantErr: true,
			errType: ErrMissingTo,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			builder := tt.setup()
			env, err := builder.Build()

			if tt.wantErr {
				assert.Error(t, err)
				if tt.errType != nil {
					assert.ErrorIs(t, err, tt.errType)
				}
				assert.Nil(t, env)
			} else {
				assert.NoError(t, err)
				assert.NotNil(t, env)
			}
		})
	}
}

func TestEnvelope_Serialize(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		Operation(OperationData).
		Capability("key", "value").
		Build()
	require.NoError(t, err)

	data, err := env.Serialize()
	assert.NoError(t, err)
	assert.NotEmpty(t, data)
}

func TestDeserializeEnvelope(t *testing.T) {
	original, err := NewEnvelope().
		From("sender").
		To("receiver").
		Operation(OperationData).
		Capability("key", "value").
		Build()
	require.NoError(t, err)

	data, err := original.Serialize()
	require.NoError(t, err)

	deserialized, err := DeserializeEnvelope(data)
	assert.NoError(t, err)
	assert.Equal(t, original.From, deserialized.From)
	assert.Equal(t, original.To, deserialized.To)
	assert.Equal(t, original.Operation, deserialized.Operation)
	assert.Equal(t, original.MessageID, deserialized.MessageID)
	assert.Equal(t, original.Capabilities["key"], deserialized.Capabilities["key"])
}

func TestEnvelope_Hash(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		Build()
	require.NoError(t, err)

	hash1 := env.Hash()
	hash2 := env.Hash()

	assert.NotEmpty(t, hash1)
	assert.Equal(t, hash1, hash2, "hash should be deterministic")
}

func TestEnvelope_Clone(t *testing.T) {
	original, err := NewEnvelope().
		From("sender").
		To("receiver").
		Operation(OperationData).
		Capability("key", "value").
		Payload([]byte("test data"), PayloadText).
		Build()
	require.NoError(t, err)

	cloned := original.Clone()

	assert.Equal(t, original.From, cloned.From)
	assert.Equal(t, original.To, cloned.To)
	assert.Equal(t, original.Operation, cloned.Operation)
	assert.Equal(t, original.MessageID, cloned.MessageID)
	assert.Equal(t, original.Capabilities, cloned.Capabilities)
	assert.Equal(t, original.Payload, cloned.Payload)

	// Verify deep copy
	cloned.Capabilities["key"] = "modified"
	assert.NotEqual(t, original.Capabilities["key"], cloned.Capabilities["key"])
}

func TestOperationType_String(t *testing.T) {
	tests := []struct {
		op   OperationType
		want string
	}{
		{OperationData, "DATA"},
		{OperationControl, "CONTROL"},
		{OperationAck, "ACK"},
		{OperationError, "ERROR"},
		{OperationRequest, "REQUEST"},
		{OperationResponse, "RESPONSE"},
	}

	for _, tt := range tests {
		t.Run(tt.want, func(t *testing.T) {
			assert.Equal(t, tt.want, tt.op.String())
		})
	}
}

func BenchmarkEnvelope_Serialize(b *testing.B) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Operation(OperationData).
		Capability("key", "value").
		Build()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = env.Serialize()
	}
}

func BenchmarkDeserializeEnvelope(b *testing.B) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Operation(OperationData).
		Build()
	data, _ := env.Serialize()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = DeserializeEnvelope(data)
	}
}
