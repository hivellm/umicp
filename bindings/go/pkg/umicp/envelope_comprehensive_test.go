package umicp

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
)

// ============================================================================
// ENVELOPE COMPREHENSIVE TESTS (Go v0.2.0)
// ============================================================================

func TestEnvelopeBasicCreation(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		Build()
	
	assert.NoError(t, err)
	assert.Equal(t, "sender", env.From)
	assert.Equal(t, "receiver", env.To)
}

func TestEnvelopeWithOperation(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		Operation(OperationData).
		Build()
	
	assert.NoError(t, err)
	assert.Equal(t, OperationData, env.Operation)
}

func TestEnvelopeWithMessageID(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		MessageID("custom-id-123").
		Build()
	
	assert.NoError(t, err)
	assert.Equal(t, "custom-id-123", env.MessageID)
}

func TestEnvelopeAutoMessageID(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		Build()
	
	assert.NoError(t, err)
	assert.NotEmpty(t, env.MessageID)
}

func TestEnvelopeSerializeDeserialize(t *testing.T) {
	original, err := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("test", 123).
		Build()
	
	assert.NoError(t, err)

	data, err := json.Marshal(original)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, original.From, decoded.From)
	assert.Equal(t, original.To, decoded.To)
}

func TestEnvelopeValidation(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		To("receiver").
		Build()
	
	assert.NoError(t, err)
	assert.NotNil(t, env)
}

func TestEnvelopeClone(t *testing.T) {
	original, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("test", 42).
		Build()

	cloned := original.Clone()

	assert.Equal(t, original.From, cloned.From)
	assert.Equal(t, original.To, cloned.To)
	assert.Equal(t, 42, cloned.Capabilities["test"])
	assert.NotSame(t, original.Capabilities, cloned.Capabilities)
}

func TestEnvelopeHash(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Build()

	hash := env.Hash()
	assert.NotEmpty(t, hash)
}

func TestEnvelopeOperationTypes(t *testing.T) {
	operations := []OperationType{
		OperationData,
		OperationControl,
		OperationAck,
		OperationError,
		OperationRequest,
		OperationResponse,
	}

	for _, op := range operations {
		env, err := NewEnvelope().
			From("sender").
			To("receiver").
			Operation(op).
			Build()
		
		assert.NoError(t, err)
		assert.Equal(t, op, env.Operation)
	}
}

func TestEnvelopeMultipleCapabilities(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("cap1", "value1").
		Capability("cap2", 100).
		Capability("cap3", true).
		Build()

	assert.Len(t, env.Capabilities, 3)
	assert.Equal(t, "value1", env.Capabilities["cap1"])
	assert.Equal(t, 100, env.Capabilities["cap2"])
	assert.Equal(t, true, env.Capabilities["cap3"])
}

func TestEnvelopeCapabilityOverwrite(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("key", "old").
		Capability("key", "new").
		Build()

	assert.Equal(t, "new", env.Capabilities["key"])
}

func TestEnvelopeEmptyFrom(t *testing.T) {
	env, err := NewEnvelope().
		To("receiver").
		Build()
	
	assert.Error(t, err) // Go validates from is required
}

func TestEnvelopeEmptyTo(t *testing.T) {
	env, err := NewEnvelope().
		From("sender").
		Build()
	
	assert.Error(t, err) // Go validates to is required
}

func TestEnvelopeDefaultOperation(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Build()

	assert.Equal(t, OperationData, env.Operation)
}

func TestEnvelopeTimestamp(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Build()

	assert.Greater(t, env.Timestamp, int64(0))
}

func TestEnvelopeJSONMarshaling(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("test", 123).
		Build()

	data, err := json.Marshal(env)
	assert.NoError(t, err)
	assert.Contains(t, string(data), "sender")
	assert.Contains(t, string(data), "receiver")
	assert.Contains(t, string(data), "test")
}

func TestEnvelopeJSONUnmarshaling(t *testing.T) {
	jsonData := `{
		"from": "sender",
		"to": "receiver",
		"operation": 1,
		"message_id": "msg-123",
		"timestamp": 1234567890,
		"capabilities": {"test": "value"}
	}`

	var env Envelope
	err := json.Unmarshal([]byte(jsonData), &env)
	assert.NoError(t, err)
	assert.Equal(t, "sender", env.From)
	assert.Equal(t, "receiver", env.To)
}

func TestEnvelopeBuilderChaining(t *testing.T) {
	env, err := NewEnvelope().
		From("s1").
		To("r1").
		Operation(OperationControl).
		MessageID("m1").
		Capability("k1", "v1").
		Capability("k2", 42).
		Build()

	assert.NoError(t, err)
	assert.Equal(t, "s1", env.From)
	assert.Equal(t, "r1", env.To)
	assert.Equal(t, OperationControl, env.Operation)
	assert.Equal(t, "m1", env.MessageID)
	assert.Len(t, env.Capabilities, 2)
}

func TestEnvelopeCapabilitiesEmpty(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Build()

	assert.NotNil(t, env.Capabilities)
	assert.Len(t, env.Capabilities, 0)
}

func TestEnvelopeCapabilitiesNil(t *testing.T) {
	caps := map[string]interface{}(nil)
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capabilities(caps).
		Build()

	assert.NotNil(t, env.Capabilities)
}

func TestEnvelopeHashConsistency(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Build()

	hash1 := env.Hash()
	hash2 := env.Hash()

	assert.Equal(t, hash1, hash2)
}

func TestEnvelopeHashUniqueness(t *testing.T) {
	env1, _ := NewEnvelope().From("sender1").To("receiver").Build()
	env2, _ := NewEnvelope().From("sender2").To("receiver").Build()

	hash1 := env1.Hash()
	hash2 := env2.Hash()

	assert.NotEqual(t, hash1, hash2)
}

func TestEnvelopeCapabilityTypes(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("int", 42).
		Capability("float", 3.14).
		Capability("bool", true).
		Capability("string", "text").
		Capability("array", []int{1, 2, 3}).
		Capability("object", map[string]int{"a": 1}).
		Capability("null", nil).
		Build()

	assert.Equal(t, 42, env.Capabilities["int"])
	assert.Equal(t, 3.14, env.Capabilities["float"])
	assert.Equal(t, true, env.Capabilities["bool"])
	assert.Equal(t, "text", env.Capabilities["string"])
	assert.NotNil(t, env.Capabilities["array"])
	assert.NotNil(t, env.Capabilities["object"])
	assert.Nil(t, env.Capabilities["null"])
}

func TestEnvelopeSerializationWithCapabilities(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("tokens", 100).
		Capability("temp", 0.7).
		Build()

	data, err := json.Marshal(env)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, float64(100), decoded.Capabilities["tokens"])
	assert.InDelta(t, 0.7, decoded.Capabilities["temp"], 0.001)
}

func TestEnvelopeCapabilitiesImmutability(t *testing.T) {
	caps := map[string]interface{}{"key": "value"}
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capabilities(caps).
		Build()

	// Modify original map
	caps["key"] = "modified"

	// Envelope should be unchanged
	assert.Equal(t, "value", env.Capabilities["key"])
}

func TestEnvelopeComplexNesting(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("config", map[string]interface{}{
			"auth": map[string]interface{}{
				"type": "oauth",
				"credentials": map[string]interface{}{
					"client_id": "abc123",
					"scopes": []string{"read", "write"},
				},
			},
			"settings": map[string]interface{}{
				"timeout": 30,
				"retry": true,
			},
		}).
		Build()

	assert.NotNil(t, env.Capabilities["config"])
	config := env.Capabilities["config"].(map[string]interface{})
	assert.NotNil(t, config["auth"])
	assert.NotNil(t, config["settings"])
}

func TestEnvelopeLargePayload(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Payload(make([]byte, 1024*100), PayloadBinary). // 100KB
		Build()

	assert.Len(t, env.Payload, 1024*100)
}

func TestEnvelopeAllOperationTypes(t *testing.T) {
	ops := map[OperationType]string{
		OperationData:     "DATA",
		OperationControl:  "CONTROL",
		OperationAck:      "ACK",
		OperationError:    "ERROR",
		OperationRequest:  "REQUEST",
		OperationResponse: "RESPONSE",
	}

	for op, name := range ops {
		env, _ := NewEnvelope().
			From("s").
			To("r").
			Operation(op).
			Build()
		
		assert.Equal(t, op, env.Operation)
		assert.Equal(t, name, op.String())
	}
}

func TestEnvelopeCapabilityStringHelper(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		CapabilityString("model", "gpt-4").
		Build()

	assert.Equal(t, "gpt-4", env.Capabilities["model"])
}

func TestEnvelopeMultipleBuilds(t *testing.T) {
	builder := NewEnvelope().From("sender").To("receiver")
	
	env1, err1 := builder.Build()
	env2, err2 := builder.Build()
	
	assert.NoError(t, err1)
	assert.NoError(t, err2)
	assert.NotSame(t, env1, env2)
}

func TestEnvelopeCapabilitiesReplacement(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("key1", "value1").
		Capabilities(map[string]interface{}{"key2": "value2"}).
		Build()

	assert.Len(t, env.Capabilities, 1)
	assert.Equal(t, "value2", env.Capabilities["key2"])
	assert.NotContains(t, env.Capabilities, "key1")
}

func TestEnvelopeJSONRoundtrip(t *testing.T) {
	original, _ := NewEnvelope().
		From("alice").
		To("bob").
		Operation(OperationRequest).
		Capability("model", "gpt-4").
		Capability("tokens", 100).
		Build()

	// Serialize
	data, err := json.Marshal(original)
	assert.NoError(t, err)

	// Deserialize
	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	// Verify
	assert.Equal(t, original.From, decoded.From)
	assert.Equal(t, original.To, decoded.To)
	assert.Equal(t, original.Operation, decoded.Operation)
	assert.Equal(t, "gpt-4", decoded.Capabilities["model"])
}

func TestEnvelopePayloadTypes(t *testing.T) {
	payloadTypes := []PayloadType{
		PayloadVector,
		PayloadText,
		PayloadMetadata,
		PayloadBinary,
	}

	for _, pt := range payloadTypes {
		env, err := NewEnvelope().
			From("sender").
			To("receiver").
			Payload([]byte("test"), pt).
			Build()
		
		assert.NoError(t, err)
		assert.Equal(t, pt, env.PayloadType)
	}
}

func TestEnvelopeEncodingTypes(t *testing.T) {
	encodings := []EncodingType{
		EncodingRaw,
		EncodingJSON,
		EncodingProtobuf,
		EncodingMsgpack,
	}

	for _, enc := range encodings {
		env, err := NewEnvelope().
			From("sender").
			To("receiver").
			Encoding(enc).
			Build()
		
		assert.NoError(t, err)
		assert.Equal(t, enc, env.Encoding)
	}
}

func TestEnvelopeUnicodeSupport(t *testing.T) {
	env, _ := NewEnvelope().
		From("发送者").
		To("接收者").
		Capability("消息", "你好世界").
		Build()

	assert.Equal(t, "发送者", env.From)
	assert.Equal(t, "接收者", env.To)
	assert.Equal(t, "你好世界", env.Capabilities["消息"])
}

func TestEnvelopeSpecialCharacters(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender!@#").
		To("receiver$%^").
		Capability("key-1", "value&*").
		Build()

	assert.Equal(t, "sender!@#", env.From)
	assert.Equal(t, "receiver$%^", env.To)
	assert.Equal(t, "value&*", env.Capabilities["key-1"])
}

func TestEnvelopeEmptyPayload(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Build()

	assert.Nil(t, env.Payload)
}

func TestEnvelopeNonEmptyPayload(t *testing.T) {
	payload := []byte("Hello, World!")
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Payload(payload, PayloadText).
		Build()

	assert.Equal(t, payload, env.Payload)
	assert.Equal(t, PayloadText, env.PayloadType)
}

func TestEnvelopeCapabilitiesWithNumbers(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("int8", int8(127)).
		Capability("int16", int16(32767)).
		Capability("int32", int32(2147483647)).
		Capability("int64", int64(9223372036854775807)).
		Capability("uint", uint(42)).
		Capability("float32", float32(3.14)).
		Capability("float64", float64(2.71828)).
		Build()

	assert.NotNil(t, env.Capabilities["int8"])
	assert.NotNil(t, env.Capabilities["int16"])
	assert.NotNil(t, env.Capabilities["int32"])
	assert.NotNil(t, env.Capabilities["int64"])
	assert.NotNil(t, env.Capabilities["uint"])
	assert.NotNil(t, env.Capabilities["float32"])
	assert.NotNil(t, env.Capabilities["float64"])
}

func TestEnvelopeCapabilitiesWithArrays(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("strings", []string{"a", "b", "c"}).
		Capability("ints", []int{1, 2, 3}).
		Capability("bools", []bool{true, false, true}).
		Build()

	assert.IsType(t, []string{}, env.Capabilities["strings"])
	assert.IsType(t, []int{}, env.Capabilities["ints"])
	assert.IsType(t, []bool{}, env.Capabilities["bools"])
}

func TestEnvelopeCapabilitiesWithMaps(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("map_string", map[string]string{"k": "v"}).
		Capability("map_int", map[string]int{"count": 10}).
		Capability("map_interface", map[string]interface{}{"mixed": 42}).
		Build()

	assert.IsType(t, map[string]string{}, env.Capabilities["map_string"])
	assert.IsType(t, map[string]int{}, env.Capabilities["map_int"])
	assert.IsType(t, map[string]interface{}{}, env.Capabilities["map_interface"])
}

func TestEnvelopeCloneIndependence(t *testing.T) {
	original, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("key", "value").
		Build()

	cloned := original.Clone()
	
	// Modify original
	original.Capabilities["key"] = "modified"

	// Clone should be unchanged
	assert.Equal(t, "value", cloned.Capabilities["key"])
}

func TestEnvelopeSerializationConsistency(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("test", 123).
		Build()

	data1, _ := json.Marshal(env)
	data2, _ := json.Marshal(env)

	assert.Equal(t, data1, data2)
}

func TestEnvelopeDeserializationWithMissingFields(t *testing.T) {
	jsonData := `{"from": "sender", "to": "receiver"}`

	var env Envelope
	err := json.Unmarshal([]byte(jsonData), &env)
	assert.NoError(t, err)
	assert.Equal(t, "sender", env.From)
	assert.Equal(t, "receiver", env.To)
}

func TestEnvelopeCapabilityNullHandling(t *testing.T) {
	env, _ := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("nullable", nil).
		Build()

	assert.Nil(t, env.Capabilities["nullable"])
}

func TestEnvelopeCapabilitiesLargeSets(t *testing.T) {
	builder := NewEnvelope().From("sender").To("receiver")
	
	for i := 0; i < 50; i++ {
		builder.Capability("key_"+string(rune('A'+i%26)), i)
	}
	
	env, _ := builder.Build()
	assert.GreaterOrEqual(t, len(env.Capabilities), 26)
}

func TestEnvelopeFromToValidation(t *testing.T) {
	// Both from and to can be empty (validated elsewhere if needed)
	env, err := NewEnvelope().Build()
	assert.NoError(t, err)
	assert.Empty(t, env.From)
	assert.Empty(t, env.To)
}

