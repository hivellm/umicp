package umicp

import (
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
)

// ============================================================================
// CAPABILITIES COMPREHENSIVE TESTS (Go v0.2.0)
// ============================================================================

func TestCapabilitiesIntegerPositive(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("max_tokens", 100).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, float64(100), decoded.Capabilities["max_tokens"])
}

func TestCapabilitiesIntegerNegative(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("offset", -50).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, float64(-50), decoded.Capabilities["offset"])
}

func TestCapabilitiesIntegerZero(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("count", 0).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, float64(0), decoded.Capabilities["count"])
}

func TestCapabilitiesFloatDecimal(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("temperature", 0.7).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.InDelta(t, 0.7, decoded.Capabilities["temperature"], 0.001)
}

func TestCapabilitiesBooleanTrue(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("enabled", true).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, true, decoded.Capabilities["enabled"])
}

func TestCapabilitiesBooleanFalse(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("disabled", false).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, false, decoded.Capabilities["disabled"])
}

func TestCapabilitiesStringSimple(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		CapabilityString("model", "gpt-4").
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, "gpt-4", decoded.Capabilities["model"])
}

func TestCapabilitiesStringEmpty(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		CapabilityString("empty", "").
		Build()

	assert.Equal(t, "", envelope.Capabilities["empty"])
}

func TestCapabilitiesStringUnicode(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		CapabilityString("message", "Hello 世界 🌍").
		Build()

	assert.Equal(t, "Hello 世界 🌍", envelope.Capabilities["message"])
}

func TestCapabilitiesArrayStrings(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("models", []string{"gpt-4", "claude-3", "llama-3"}).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	arr, ok := decoded.Capabilities["models"].([]interface{})
	assert.True(t, ok)
	assert.Equal(t, 3, len(arr))
	assert.Equal(t, "gpt-4", arr[0])
}

func TestCapabilitiesArrayIntegers(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("limits", []int{10, 20, 30, 40}).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	arr, ok := decoded.Capabilities["limits"].([]interface{})
	assert.True(t, ok)
	assert.Equal(t, 4, len(arr))
}

func TestCapabilitiesArrayEmpty(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("empty_array", []interface{}{}).
		Build()

	arr, ok := envelope.Capabilities["empty_array"].([]interface{})
	assert.True(t, ok)
	assert.Equal(t, 0, len(arr))
}

func TestCapabilitiesObjectSimple(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("config", map[string]interface{}{
			"timeout": 30,
			"retries": 3,
		}).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	obj, ok := decoded.Capabilities["config"].(map[string]interface{})
	assert.True(t, ok)
	assert.Equal(t, float64(30), obj["timeout"])
}

func TestCapabilitiesObjectNested(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("auth", map[string]interface{}{
			"type": "oauth",
			"credentials": map[string]interface{}{
				"client_id": "abc123",
				"scope":     []string{"read", "write"},
			},
		}).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	auth, ok := decoded.Capabilities["auth"].(map[string]interface{})
	assert.True(t, ok)
	assert.Equal(t, "oauth", auth["type"])
}

func TestCapabilitiesNullValue(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("optional", nil).
		Build()

	assert.Nil(t, envelope.Capabilities["optional"])
}

func TestCapabilitiesComplexMixed(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		CapabilityString("string_val", "test").
		Capability("int_val", 42).
		Capability("float_val", 3.14).
		Capability("bool_val", true).
		Capability("null_val", nil).
		Capability("array_val", []int{1, 2, 3}).
		Capability("object_val", map[string]string{"key": "value"}).
		Build()

	assert.Equal(t, "test", envelope.Capabilities["string_val"])
	assert.Equal(t, 42, envelope.Capabilities["int_val"])
	assert.Equal(t, 3.14, envelope.Capabilities["float_val"])
	assert.Equal(t, true, envelope.Capabilities["bool_val"])
	assert.Nil(t, envelope.Capabilities["null_val"])
}

func TestCapabilitiesSpecialChars(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("key-dash", 1).
		Capability("key_underscore", 2).
		Capability("key.dot", 3).
		Build()

	assert.Equal(t, 1, envelope.Capabilities["key-dash"])
	assert.Equal(t, 2, envelope.Capabilities["key_underscore"])
	assert.Equal(t, 3, envelope.Capabilities["key.dot"])
}

func TestCapabilitiesLargeObject(t *testing.T) {
	builder := NewEnvelope().From("sender").To("receiver")
	
	for i := 0; i < 100; i++ {
		builder.Capability(string(rune('a'+i/26))+string(rune('a'+i%26)), i)
	}
	
	envelope := builder.Build()
	assert.Equal(t, 100, len(envelope.Capabilities))
}

func TestCapabilitiesTypeChecking(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("int", 42).
		Capability("float", 3.14).
		Capability("bool", true).
		CapabilityString("string", "text").
		Capability("array", []int{1, 2}).
		Capability("object", map[string]int{"a": 1}).
		Build()

	_, isInt := envelope.Capabilities["int"].(int)
	_, isFloat := envelope.Capabilities["float"].(float64)
	_, isBool := envelope.Capabilities["bool"].(bool)
	_, isString := envelope.Capabilities["string"].(string)

	assert.True(t, isInt)
	assert.True(t, isFloat)
	assert.True(t, isBool)
	assert.True(t, isString)
}

func TestCapabilitiesOverwrite(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("key", 1).
		Capability("key", 2). // Overwrite
		Build()

	assert.Equal(t, 2, envelope.Capabilities["key"])
}

func TestCapabilitiesClone(t *testing.T) {
	original := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("test", 42).
		Build()

	cloned := original.Clone()

	assert.Equal(t, 42, cloned.Capabilities["test"])
	assert.NotSame(t, original.Capabilities, cloned.Capabilities)
}

func TestCapabilitiesSerialization(t *testing.T) {
	envelope := NewEnvelope().
		From("sender").
		To("receiver").
		Capability("count", 100).
		Capability("enabled", true).
		Capability("ratio", 0.75).
		Build()

	data, err := json.Marshal(envelope)
	assert.NoError(t, err)

	var decoded Envelope
	err = json.Unmarshal(data, &decoded)
	assert.NoError(t, err)

	assert.Equal(t, "sender", decoded.From)
	assert.Equal(t, "receiver", decoded.To)
	assert.NotNil(t, decoded.Capabilities)
}

