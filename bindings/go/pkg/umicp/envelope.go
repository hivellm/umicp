package umicp

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"time"

	"github.com/google/uuid"
)

// Envelope represents a UMICP message envelope
type Envelope struct {
	From         string                 `json:"from"`
	To           string                 `json:"to"`
	Operation    OperationType          `json:"operation"`
	MessageID    string                 `json:"message_id"`
	Timestamp    int64                  `json:"timestamp"`
	Capabilities map[string]interface{} `json:"capabilities"`
	Payload      []byte                 `json:"payload,omitempty"`
	PayloadType  PayloadType            `json:"payload_type,omitempty"`
	Encoding     EncodingType           `json:"encoding,omitempty"`
}

// EnvelopeBuilder provides a builder pattern for creating envelopes
type EnvelopeBuilder struct {
	envelope *Envelope
}

// NewEnvelope creates a new envelope builder with default values
func NewEnvelope() *EnvelopeBuilder {
	return &EnvelopeBuilder{
		envelope: &Envelope{
			MessageID:    uuid.New().String(),
			Timestamp:    time.Now().UnixMilli(),
			Capabilities: make(map[string]interface{}),
			Operation:    OperationData,
		},
	}
}

// From sets the sender ID
func (b *EnvelopeBuilder) From(from string) *EnvelopeBuilder {
	b.envelope.From = from
	return b
}

// To sets the recipient ID
func (b *EnvelopeBuilder) To(to string) *EnvelopeBuilder {
	b.envelope.To = to
	return b
}

// Operation sets the operation type
func (b *EnvelopeBuilder) Operation(op OperationType) *EnvelopeBuilder {
	b.envelope.Operation = op
	return b
}

// MessageID sets a custom message ID
func (b *EnvelopeBuilder) MessageID(id string) *EnvelopeBuilder {
	b.envelope.MessageID = id
	return b
}

// Capability adds a single capability with any type
func (b *EnvelopeBuilder) Capability(key string, value interface{}) *EnvelopeBuilder {
	b.envelope.Capabilities[key] = value
	return b
}

// CapabilityString adds a string capability (helper)
func (b *EnvelopeBuilder) CapabilityString(key, value string) *EnvelopeBuilder {
	b.envelope.Capabilities[key] = value
	return b
}

// Capabilities sets all capabilities at once
func (b *EnvelopeBuilder) Capabilities(caps map[string]interface{}) *EnvelopeBuilder {
	b.envelope.Capabilities = caps
	return b
}

// Payload sets the payload data and type
func (b *EnvelopeBuilder) Payload(data []byte, payloadType PayloadType) *EnvelopeBuilder {
	b.envelope.Payload = data
	b.envelope.PayloadType = payloadType
	return b
}

// Encoding sets the encoding type
func (b *EnvelopeBuilder) Encoding(encoding EncodingType) *EnvelopeBuilder {
	b.envelope.Encoding = encoding
	return b
}

// Build validates and returns the constructed envelope
func (b *EnvelopeBuilder) Build() (*Envelope, error) {
	if err := b.envelope.Validate(); err != nil {
		return nil, err
	}
	return b.envelope, nil
}

// Validate checks if the envelope has all required fields
func (e *Envelope) Validate() error {
	if e.From == "" {
		return ErrMissingFrom
	}
	if e.To == "" {
		return ErrMissingTo
	}
	if e.MessageID == "" {
		return ErrMissingMessageID
	}
	return nil
}

// Hash generates a SHA256 hash of the envelope
func (e *Envelope) Hash() string {
	h := sha256.New()
	h.Write([]byte(e.From + e.To + e.MessageID))
	return hex.EncodeToString(h.Sum(nil))
}

// Serialize converts the envelope to JSON bytes
func (e *Envelope) Serialize() ([]byte, error) {
	data, err := json.Marshal(e)
	if err != nil {
		return nil, ErrSerializationFailed
	}
	return data, nil
}

// DeserializeEnvelope creates an envelope from JSON bytes
func DeserializeEnvelope(data []byte) (*Envelope, error) {
	var env Envelope
	if err := json.Unmarshal(data, &env); err != nil {
		return nil, ErrDeserializationFailed
	}
	if err := env.Validate(); err != nil {
		return nil, err
	}
	return &env, nil
}

// Clone creates a deep copy of the envelope
func (e *Envelope) Clone() *Envelope {
	clone := &Envelope{
		From:         e.From,
		To:           e.To,
		Operation:    e.Operation,
		MessageID:    e.MessageID,
		Timestamp:    e.Timestamp,
		Capabilities: make(map[string]interface{}),
		PayloadType:  e.PayloadType,
		Encoding:     e.Encoding,
	}

	// Deep copy capabilities
	for k, v := range e.Capabilities {
		clone.Capabilities[k] = v
	}

	// Deep copy payload
	if e.Payload != nil {
		clone.Payload = make([]byte, len(e.Payload))
		copy(clone.Payload, e.Payload)
	}

	return clone
}
