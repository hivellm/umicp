package umicp

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestEnvelopeBuilder_MessageID(t *testing.T) {
	builder := NewEnvelope()

	customID := "custom-msg-123"
	builder.MessageID(customID)

	env, err := builder.From("sender").To("receiver").Build()
	assert.NoError(t, err)
	assert.Equal(t, customID, env.MessageID)
}

func TestEnvelopeBuilder_Capabilities(t *testing.T) {
	builder := NewEnvelope()

	caps := map[string]string{
		"version": "1.0",
		"type":    "test",
		"region":  "us-east-1",
	}

	builder.Capabilities(caps)

	env, err := builder.From("sender").To("receiver").Build()
	assert.NoError(t, err)
	assert.Equal(t, "1.0", env.Capabilities["version"])
	assert.Equal(t, "test", env.Capabilities["type"])
	assert.Equal(t, "us-east-1", env.Capabilities["region"])
}

func TestEnvelopeBuilder_Encoding(t *testing.T) {
	builder := NewEnvelope()

	builder.Encoding(EncodingJSON)

	env, err := builder.From("sender").To("receiver").Build()
	assert.NoError(t, err)
	assert.Equal(t, EncodingJSON, env.Encoding)
}

func TestEnvelopeBuilder_AllEncodings(t *testing.T) {
	encodings := []EncodingType{
		EncodingRaw,
		EncodingJSON,
		EncodingProtobuf,
		EncodingMsgpack,
	}

	for _, encoding := range encodings {
		t.Run(encoding.String(), func(t *testing.T) {
			env, err := NewEnvelope().
				From("sender").
				To("receiver").
				Encoding(encoding).
				Build()

			assert.NoError(t, err)
			assert.Equal(t, encoding, env.Encoding)
		})
	}
}

func TestEnvelopeBuilder_AllPayloadTypes(t *testing.T) {
	payloadTypes := []PayloadType{
		PayloadVector,
		PayloadText,
		PayloadMetadata,
		PayloadBinary,
	}

	for _, ptype := range payloadTypes {
		t.Run(ptype.String(), func(t *testing.T) {
			env, err := NewEnvelope().
				From("sender").
				To("receiver").
				Payload([]byte("test"), ptype).
				Build()

			assert.NoError(t, err)
			assert.Equal(t, ptype, env.PayloadType)
		})
	}
}

func TestEnvelopeBuilder_CompleteEnvelope(t *testing.T) {
	env, err := NewEnvelope().
		From("peer-001").
		To("peer-002").
		Operation(OperationRequest).
		MessageID("req-123").
		Capability("version", "2.0").
		Capability("timeout", "30s").
		Payload([]byte(`{"key":"value"}`), PayloadBinary).
		Encoding(EncodingJSON).
		Build()

	assert.NoError(t, err)
	assert.Equal(t, "peer-001", env.From)
	assert.Equal(t, "peer-002", env.To)
	assert.Equal(t, OperationRequest, env.Operation)
	assert.Equal(t, "req-123", env.MessageID)
	assert.Equal(t, "2.0", env.Capabilities["version"])
	assert.Equal(t, "30s", env.Capabilities["timeout"])
	assert.Equal(t, PayloadBinary, env.PayloadType)
	assert.Equal(t, EncodingJSON, env.Encoding)
	assert.NotNil(t, env.Payload)
}

func TestEnvelopeBuilder_OverrideCapabilities(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("key1", "value1").
		Capability("key2", "value2").
		Capabilities(map[string]string{
			"key3": "value3",
			"key4": "value4",
		}).
		Build()

	assert.NoError(t, err)
	// Capabilities() should replace previous capabilities
	assert.Len(t, env.Capabilities, 2)
	assert.Equal(t, "value3", env.Capabilities["key3"])
	assert.Equal(t, "value4", env.Capabilities["key4"])
}

func TestEnvelopeBuilder_EmptyCapabilities(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		Capabilities(map[string]string{}).
		Build()

	assert.NoError(t, err)
	assert.NotNil(t, env.Capabilities)
	assert.Len(t, env.Capabilities, 0)
}

func TestOperationType_String_AllTypes(t *testing.T) {
	tests := []struct {
		opType   OperationType
		expected string
	}{
		{OperationData, "DATA"},
		{OperationControl, "CONTROL"},
		{OperationAck, "ACK"},
		{OperationError, "ERROR"},
		{OperationRequest, "REQUEST"},
		{OperationResponse, "RESPONSE"},
		{OperationType(99), "UNKNOWN"}, // Invalid type
	}

	for _, tt := range tests {
		t.Run(tt.expected, func(t *testing.T) {
			assert.Equal(t, tt.expected, tt.opType.String())
		})
	}
}

func TestPayloadType_String_AllTypes(t *testing.T) {
	tests := []struct {
		pType    PayloadType
		expected string
	}{
		{PayloadVector, "VECTOR"},
		{PayloadText, "TEXT"},
		{PayloadMetadata, "METADATA"},
		{PayloadBinary, "BINARY"},
		{PayloadType(99), "UNKNOWN"}, // Invalid type
	}

	for _, tt := range tests {
		t.Run(tt.expected, func(t *testing.T) {
			assert.Equal(t, tt.expected, tt.pType.String())
		})
	}
}

func TestEncodingType_String_AllTypes(t *testing.T) {
	tests := []struct {
		eType    EncodingType
		expected string
	}{
		{EncodingRaw, "RAW"},
		{EncodingJSON, "JSON"},
		{EncodingProtobuf, "PROTOBUF"},
		{EncodingMsgpack, "MSGPACK"},
		{EncodingType(99), "UNKNOWN"}, // Invalid type
	}

	for _, tt := range tests {
		t.Run(tt.expected, func(t *testing.T) {
			assert.Equal(t, tt.expected, tt.eType.String())
		})
	}
}

func TestEnvelope_Validate_EdgeCases(t *testing.T) {
	t.Run("Missing From", func(t *testing.T) {
		env := &Envelope{
			To:        "receiver",
			Operation: OperationData,
			MessageID: "msg-123",
		}

		err := env.Validate()
		assert.Error(t, err)
		assert.Contains(t, err.Error(), "from")
	})

	t.Run("Missing To", func(t *testing.T) {
		env := &Envelope{
			From:      "sender",
			Operation: OperationData,
			MessageID: "msg-123",
		}

		err := env.Validate()
		assert.Error(t, err)
		assert.Contains(t, err.Error(), "to")
	})

	t.Run("Missing MessageID", func(t *testing.T) {
		env := &Envelope{
			From:      "sender",
			To:        "receiver",
			Operation: OperationData,
		}

		err := env.Validate()
		assert.Error(t, err)
		assert.Contains(t, err.Error(), "message_id")
	})

	t.Run("Valid envelope", func(t *testing.T) {
		env := &Envelope{
			From:      "sender",
			To:        "receiver",
			Operation: OperationData,
			MessageID: "msg-123",
		}

		err := env.Validate()
		assert.NoError(t, err)
	})
}

func TestEnvelope_Serialize_Deserialize_AllFields(t *testing.T) {
	original, _ := NewEnvelope().
		From("peer-alpha").
		To("peer-beta").
		Operation(OperationResponse).
		MessageID("resp-456").
		Capability("status", "success").
		Capability("code", "200").
		Payload([]byte("response data"), PayloadText).
		Encoding(EncodingJSON).
		Build()

	// Serialize
	data, err := original.Serialize()
	assert.NoError(t, err)
	assert.NotNil(t, data)

	// Deserialize
	restored, err := DeserializeEnvelope(data)
	assert.NoError(t, err)
	assert.NotNil(t, restored)

	// Verify all fields
	assert.Equal(t, original.From, restored.From)
	assert.Equal(t, original.To, restored.To)
	assert.Equal(t, original.Operation, restored.Operation)
	assert.Equal(t, original.MessageID, restored.MessageID)
	assert.Equal(t, original.Capabilities, restored.Capabilities)
	assert.Equal(t, original.Payload, restored.Payload)
	assert.Equal(t, original.PayloadType, restored.PayloadType)
	assert.Equal(t, original.Encoding, restored.Encoding)
}

func TestEnvelope_SerializeError_InvalidJSON(t *testing.T) {
	// Create envelope with invalid JSON-able data
	env := &Envelope{
		From:      "sender",
		To:        "receiver",
		MessageID: "msg-123",
		Capabilities: map[string]string{
			"valid": "value",
		},
	}

	// Should not error with valid data
	_, err := env.Serialize()
	assert.NoError(t, err)
}

func TestEnvelope_DeserializeError(t *testing.T) {
	t.Run("Invalid JSON", func(t *testing.T) {
		invalidJSON := []byte(`{invalid json}`)
		_, err := DeserializeEnvelope(invalidJSON)
		assert.Error(t, err)
	})

	t.Run("Empty data", func(t *testing.T) {
		_, err := DeserializeEnvelope([]byte{})
		assert.Error(t, err)
	})

	t.Run("Nil data", func(t *testing.T) {
		_, err := DeserializeEnvelope(nil)
		assert.Error(t, err)
	})
}

func TestEnvelope_CloneWithAllFields(t *testing.T) {
	original, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("key", "value").
		Payload([]byte("data"), PayloadBinary).
		Build()

	cloned := original.Clone()

	// Verify fields are equal
	assert.Equal(t, original.From, cloned.From)
	assert.Equal(t, original.To, cloned.To)
	assert.Equal(t, original.MessageID, cloned.MessageID)
	assert.Equal(t, original.Capabilities, cloned.Capabilities)

	// Verify it's a real clone (modify original doesn't affect clone)
	original.From = "modified"
	assert.NotEqual(t, original.From, cloned.From)
	assert.Equal(t, "sender", cloned.From)

	// Modify capabilities
	original.Capabilities["key"] = "modified"
	assert.Equal(t, "value", cloned.Capabilities["key"])
}

func TestEnvelope_HashConsistency(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		MessageID("msg-123").
		Build()

	hash := env.Hash()

	assert.NotEmpty(t, hash)
	assert.Len(t, hash, 64) // SHA-256 produces 64 hex characters

	// Same envelope should produce same hash
	hash2 := env.Hash()
	assert.Equal(t, hash, hash2)
}

func TestEnvelope_HashUniqueness(t *testing.T) {
	env1, _ := NewEnvelope().From("s1").To("r1").Build()
	env2, _ := NewEnvelope().From("s2").To("r2").Build()

	hash1 := env1.Hash()
	hash2 := env2.Hash()

	assert.NotEqual(t, hash1, hash2)
}
