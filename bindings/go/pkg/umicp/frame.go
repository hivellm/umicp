package umicp

import (
	"encoding/binary"
	"fmt"
	"io"
)

const (
	// FrameHeaderSize is the size of the frame header in bytes
	FrameHeaderSize = 8

	// MaxFrameSize is the maximum allowed frame size (100MB)
	MaxFrameSize = 100 * 1024 * 1024
)

// FrameType defines the type of frame
type FrameType uint16

const (
	// FrameTypeData represents a data frame
	FrameTypeData FrameType = 0

	// FrameTypeControl represents a control frame
	FrameTypeControl FrameType = 1

	// FrameTypePing represents a ping frame
	FrameTypePing FrameType = 2

	// FrameTypePong represents a pong frame
	FrameTypePong FrameType = 3
)

// Frame represents a UMICP protocol frame
type Frame struct {
	Length   uint32    // Payload length
	Type     FrameType // Frame type
	Flags    uint8     // Frame flags
	Reserved uint8     // Reserved for future use
	Payload  []byte    // Frame payload
}

// NewFrame creates a new frame with the given type and payload
func NewFrame(frameType FrameType, payload []byte) *Frame {
	return &Frame{
		Length:   uint32(len(payload)),
		Type:     frameType,
		Flags:    0,
		Reserved: 0,
		Payload:  payload,
	}
}

// NewDataFrame creates a new data frame with the given payload
func NewDataFrame(payload []byte) *Frame {
	return NewFrame(FrameTypeData, payload)
}

// NewControlFrame creates a new control frame with the given payload
func NewControlFrame(payload []byte) *Frame {
	return NewFrame(FrameTypeControl, payload)
}

// NewPingFrame creates a new ping frame
func NewPingFrame() *Frame {
	return NewFrame(FrameTypePing, nil)
}

// NewPongFrame creates a new pong frame
func NewPongFrame() *Frame {
	return NewFrame(FrameTypePong, nil)
}

// Serialize writes the frame to the given writer
func (f *Frame) Serialize(w io.Writer) error {
	// Validate frame size
	if f.Length > MaxFrameSize {
		return ErrFrameTooLarge
	}

	// Create header
	header := make([]byte, FrameHeaderSize)
	binary.BigEndian.PutUint32(header[0:4], f.Length)
	binary.BigEndian.PutUint16(header[4:6], uint16(f.Type))
	header[6] = f.Flags
	header[7] = f.Reserved

	// Write header
	if _, err := w.Write(header); err != nil {
		return fmt.Errorf("failed to write frame header: %w", err)
	}

	// Write payload if present
	if f.Length > 0 && f.Payload != nil {
		if _, err := w.Write(f.Payload); err != nil {
			return fmt.Errorf("failed to write frame payload: %w", err)
		}
	}

	return nil
}

// DeserializeFrame reads a frame from the given reader
func DeserializeFrame(r io.Reader) (*Frame, error) {
	// Read header
	header := make([]byte, FrameHeaderSize)
	if _, err := io.ReadFull(r, header); err != nil {
		if err == io.EOF {
			return nil, err
		}
		return nil, fmt.Errorf("failed to read frame header: %w", err)
	}

	// Parse header
	frame := &Frame{
		Length:   binary.BigEndian.Uint32(header[0:4]),
		Type:     FrameType(binary.BigEndian.Uint16(header[4:6])),
		Flags:    header[6],
		Reserved: header[7],
	}

	// Validate frame size
	if frame.Length > MaxFrameSize {
		return nil, ErrFrameTooLarge
	}

	// Read payload if present
	if frame.Length > 0 {
		frame.Payload = make([]byte, frame.Length)
		if _, err := io.ReadFull(r, frame.Payload); err != nil {
			return nil, fmt.Errorf("failed to read frame payload: %w", err)
		}
	}

	return frame, nil
}

// IsData returns true if this is a data frame
func (f *Frame) IsData() bool {
	return f.Type == FrameTypeData
}

// IsControl returns true if this is a control frame
func (f *Frame) IsControl() bool {
	return f.Type == FrameTypeControl
}

// IsPing returns true if this is a ping frame
func (f *Frame) IsPing() bool {
	return f.Type == FrameTypePing
}

// IsPong returns true if this is a pong frame
func (f *Frame) IsPong() bool {
	return f.Type == FrameTypePong
}

// String returns a string representation of the frame type
func (ft FrameType) String() string {
	switch ft {
	case FrameTypeData:
		return "DATA"
	case FrameTypeControl:
		return "CONTROL"
	case FrameTypePing:
		return "PING"
	case FrameTypePong:
		return "PONG"
	default:
		return fmt.Sprintf("UNKNOWN(%d)", ft)
	}
}
