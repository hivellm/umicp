package umicp

// OperationType defines the type of operation in an envelope
type OperationType uint8

const (
	// OperationData represents a regular data message
	OperationData OperationType = iota
	// OperationControl represents a control/management message
	OperationControl
	// OperationAck represents an acknowledgment message
	OperationAck
	// OperationError represents an error message
	OperationError
	// OperationRequest represents a request message
	OperationRequest
	// OperationResponse represents a response message
	OperationResponse
)

// String returns the string representation of the operation type
func (o OperationType) String() string {
	switch o {
	case OperationData:
		return "DATA"
	case OperationControl:
		return "CONTROL"
	case OperationAck:
		return "ACK"
	case OperationError:
		return "ERROR"
	case OperationRequest:
		return "REQUEST"
	case OperationResponse:
		return "RESPONSE"
	default:
		return "UNKNOWN"
	}
}

// PayloadType defines the type of payload in an envelope
type PayloadType uint8

const (
	// PayloadVector represents vector/embedding data
	PayloadVector PayloadType = iota
	// PayloadText represents text data
	PayloadText
	// PayloadMetadata represents metadata
	PayloadMetadata
	// PayloadBinary represents binary data
	PayloadBinary
)

// String returns the string representation of the payload type
func (p PayloadType) String() string {
	switch p {
	case PayloadVector:
		return "VECTOR"
	case PayloadText:
		return "TEXT"
	case PayloadMetadata:
		return "METADATA"
	case PayloadBinary:
		return "BINARY"
	default:
		return "UNKNOWN"
	}
}

// EncodingType defines the encoding used for the payload
type EncodingType uint8

const (
	// EncodingRaw represents raw/unencoded data
	EncodingRaw EncodingType = iota
	// EncodingJSON represents JSON encoding
	EncodingJSON
	// EncodingProtobuf represents Protocol Buffers encoding
	EncodingProtobuf
	// EncodingMsgpack represents MessagePack encoding
	EncodingMsgpack
)

// String returns the string representation of the encoding type
func (e EncodingType) String() string {
	switch e {
	case EncodingRaw:
		return "RAW"
	case EncodingJSON:
		return "JSON"
	case EncodingProtobuf:
		return "PROTOBUF"
	case EncodingMsgpack:
		return "MSGPACK"
	default:
		return "UNKNOWN"
	}
}
