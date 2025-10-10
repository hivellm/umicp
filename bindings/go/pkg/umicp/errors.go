package umicp

import "errors"

// Envelope errors
var (
	// ErrMissingFrom indicates the 'from' field is missing
	ErrMissingFrom = errors.New("envelope: missing 'from' field")

	// ErrMissingTo indicates the 'to' field is missing
	ErrMissingTo = errors.New("envelope: missing 'to' field")

	// ErrMissingMessageID indicates the 'message_id' field is missing
	ErrMissingMessageID = errors.New("envelope: missing 'message_id' field")

	// ErrInvalidEnvelope indicates the envelope is invalid
	ErrInvalidEnvelope = errors.New("envelope: invalid envelope")

	// ErrSerializationFailed indicates serialization failed
	ErrSerializationFailed = errors.New("envelope: serialization failed")

	// ErrDeserializationFailed indicates deserialization failed
	ErrDeserializationFailed = errors.New("envelope: deserialization failed")
)

// Transport errors
var (
	// ErrNotConnected indicates transport is not connected
	ErrNotConnected = errors.New("transport: not connected")

	// ErrConnectionClosed indicates the connection is closed
	ErrConnectionClosed = errors.New("transport: connection closed")

	// ErrTimeout indicates the operation timed out
	ErrTimeout = errors.New("transport: operation timeout")

	// ErrConnectionFailed indicates connection attempt failed
	ErrConnectionFailed = errors.New("transport: connection failed")

	// ErrSendFailed indicates message send failed
	ErrSendFailed = errors.New("transport: send failed")

	// ErrReceiveFailed indicates message receive failed
	ErrReceiveFailed = errors.New("transport: receive failed")
)

// Peer errors
var (
	// ErrPeerNotFound indicates the peer was not found
	ErrPeerNotFound = errors.New("peer: peer not found")

	// ErrHandshakeTimeout indicates handshake timed out
	ErrHandshakeTimeout = errors.New("peer: handshake timeout")

	// ErrAlreadyConnected indicates already connected to peer
	ErrAlreadyConnected = errors.New("peer: already connected")

	// ErrPeerDisconnected indicates peer is disconnected
	ErrPeerDisconnected = errors.New("peer: peer disconnected")

	// ErrInvalidPeerConfig indicates invalid peer configuration
	ErrInvalidPeerConfig = errors.New("peer: invalid configuration")
)

// Frame errors
var (
	// ErrFrameTooLarge indicates frame exceeds maximum size
	ErrFrameTooLarge = errors.New("frame: size exceeds maximum")

	// ErrInvalidFrameHeader indicates invalid frame header
	ErrInvalidFrameHeader = errors.New("frame: invalid header")

	// ErrInvalidFramePayload indicates invalid frame payload
	ErrInvalidFramePayload = errors.New("frame: invalid payload")
)
