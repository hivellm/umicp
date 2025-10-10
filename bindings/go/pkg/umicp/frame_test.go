package umicp

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNewFrame(t *testing.T) {
	payload := []byte("test data")
	frame := NewFrame(FrameTypeData, payload)

	assert.Equal(t, uint32(len(payload)), frame.Length)
	assert.Equal(t, FrameTypeData, frame.Type)
	assert.Equal(t, payload, frame.Payload)
}

func TestNewDataFrame(t *testing.T) {
	payload := []byte("test data")
	frame := NewDataFrame(payload)

	assert.Equal(t, FrameTypeData, frame.Type)
	assert.Equal(t, payload, frame.Payload)
}

func TestFrame_Serialize(t *testing.T) {
	tests := []struct {
		name    string
		frame   *Frame
		wantErr bool
	}{
		{
			name:    "data frame with payload",
			frame:   NewDataFrame([]byte("test")),
			wantErr: false,
		},
		{
			name:    "ping frame without payload",
			frame:   NewPingFrame(),
			wantErr: false,
		},
		{
			name: "frame too large",
			frame: &Frame{
				Length:  MaxFrameSize + 1,
				Type:    FrameTypeData,
				Payload: make([]byte, MaxFrameSize+1),
			},
			wantErr: true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			var buf bytes.Buffer
			err := tt.frame.Serialize(&buf)

			if tt.wantErr {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
				assert.NotEmpty(t, buf.Bytes())
				assert.GreaterOrEqual(t, buf.Len(), FrameHeaderSize)
			}
		})
	}
}

func TestDeserializeFrame(t *testing.T) {
	payload := []byte("test data")
	original := NewDataFrame(payload)

	var buf bytes.Buffer
	err := original.Serialize(&buf)
	require.NoError(t, err)

	deserialized, err := DeserializeFrame(&buf)
	assert.NoError(t, err)
	assert.Equal(t, original.Length, deserialized.Length)
	assert.Equal(t, original.Type, deserialized.Type)
	assert.Equal(t, original.Payload, deserialized.Payload)
}

func TestFrame_TypeChecks(t *testing.T) {
	tests := []struct {
		name      string
		frame     *Frame
		isData    bool
		isControl bool
		isPing    bool
		isPong    bool
	}{
		{
			name:      "data frame",
			frame:     NewDataFrame([]byte("test")),
			isData:    true,
			isControl: false,
			isPing:    false,
			isPong:    false,
		},
		{
			name:      "control frame",
			frame:     NewControlFrame([]byte("control")),
			isData:    false,
			isControl: true,
			isPing:    false,
			isPong:    false,
		},
		{
			name:      "ping frame",
			frame:     NewPingFrame(),
			isData:    false,
			isControl: false,
			isPing:    true,
			isPong:    false,
		},
		{
			name:      "pong frame",
			frame:     NewPongFrame(),
			isData:    false,
			isControl: false,
			isPing:    false,
			isPong:    true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			assert.Equal(t, tt.isData, tt.frame.IsData())
			assert.Equal(t, tt.isControl, tt.frame.IsControl())
			assert.Equal(t, tt.isPing, tt.frame.IsPing())
			assert.Equal(t, tt.isPong, tt.frame.IsPong())
		})
	}
}

func TestFrameType_String(t *testing.T) {
	tests := []struct {
		frameType FrameType
		want      string
	}{
		{FrameTypeData, "DATA"},
		{FrameTypeControl, "CONTROL"},
		{FrameTypePing, "PING"},
		{FrameTypePong, "PONG"},
	}

	for _, tt := range tests {
		t.Run(tt.want, func(t *testing.T) {
			assert.Equal(t, tt.want, tt.frameType.String())
		})
	}
}

func TestFrame_RoundTrip(t *testing.T) {
	payloads := [][]byte{
		nil,
		[]byte{},
		[]byte("test"),
		[]byte("longer test data with multiple words"),
		make([]byte, 1024), // 1KB
	}

	for _, payload := range payloads {
		t.Run("", func(t *testing.T) {
			original := NewDataFrame(payload)

			var buf bytes.Buffer
			err := original.Serialize(&buf)
			require.NoError(t, err)

			deserialized, err := DeserializeFrame(&buf)
			require.NoError(t, err)

			assert.Equal(t, original.Length, deserialized.Length)
			assert.Equal(t, original.Type, deserialized.Type)
			assert.Equal(t, original.Flags, deserialized.Flags)

			// For zero-length payloads, both nil and empty slice are acceptable
			if original.Length == 0 {
				assert.True(t, len(deserialized.Payload) == 0,
					"Zero-length payload should be empty, got len=%d", len(deserialized.Payload))
			} else {
				assert.Equal(t, original.Payload, deserialized.Payload)
			}
		})
	}
}

func BenchmarkFrame_Serialize(b *testing.B) {
	frame := NewDataFrame([]byte("test data"))
	var buf bytes.Buffer

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		buf.Reset()
		_ = frame.Serialize(&buf)
	}
}

func BenchmarkDeserializeFrame(b *testing.B) {
	frame := NewDataFrame([]byte("test data"))
	var buf bytes.Buffer
	_ = frame.Serialize(&buf)
	data := buf.Bytes()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		reader := bytes.NewReader(data)
		_, _ = DeserializeFrame(reader)
	}
}
