package umicp

import (
	"testing"
)

func TestEnvelopeErrors(t *testing.T) {
	t.Run("Missing from error", func(t *testing.T) {
		if ErrMissingFrom == nil {
			t.Error("ErrMissingFrom should not be nil")
		}
		if ErrMissingFrom.Error() != "envelope: missing 'from' field" {
			t.Error("Error message incorrect")
		}
	})

	t.Run("Missing to error", func(t *testing.T) {
		if ErrMissingTo == nil {
			t.Error("ErrMissingTo should not be nil")
		}
	})

	t.Run("Missing message ID error", func(t *testing.T) {
		if ErrMissingMessageID == nil {
			t.Error("ErrMissingMessageID should not be nil")
		}
	})

	t.Run("Invalid envelope error", func(t *testing.T) {
		if ErrInvalidEnvelope == nil {
			t.Error("ErrInvalidEnvelope should not be nil")
		}
	})

	t.Run("Serialization failed error", func(t *testing.T) {
		if ErrSerializationFailed == nil {
			t.Error("ErrSerializationFailed should not be nil")
		}
	})

	t.Run("Deserialization failed error", func(t *testing.T) {
		if ErrDeserializationFailed == nil {
			t.Error("ErrDeserializationFailed should not be nil")
		}
	})
}

func TestTransportErrors(t *testing.T) {
	t.Run("Not connected error", func(t *testing.T) {
		if ErrNotConnected == nil {
			t.Error("ErrNotConnected should not be nil")
		}
	})

	t.Run("Connection closed error", func(t *testing.T) {
		if ErrConnectionClosed == nil {
			t.Error("ErrConnectionClosed should not be nil")
		}
	})

	t.Run("Timeout error", func(t *testing.T) {
		if ErrTimeout == nil {
			t.Error("ErrTimeout should not be nil")
		}
	})

	t.Run("Connection failed error", func(t *testing.T) {
		if ErrConnectionFailed == nil {
			t.Error("ErrConnectionFailed should not be nil")
		}
	})

	t.Run("Send failed error", func(t *testing.T) {
		if ErrSendFailed == nil {
			t.Error("ErrSendFailed should not be nil")
		}
	})

	t.Run("Receive failed error", func(t *testing.T) {
		if ErrReceiveFailed == nil {
			t.Error("ErrReceiveFailed should not be nil")
		}
	})
}

func TestPeerErrors(t *testing.T) {
	t.Run("Peer not found error", func(t *testing.T) {
		if ErrPeerNotFound == nil {
			t.Error("ErrPeerNotFound should not be nil")
		}
	})

	t.Run("Handshake timeout error", func(t *testing.T) {
		if ErrHandshakeTimeout == nil {
			t.Error("ErrHandshakeTimeout should not be nil")
		}
	})

	t.Run("Already connected error", func(t *testing.T) {
		if ErrAlreadyConnected == nil {
			t.Error("ErrAlreadyConnected should not be nil")
		}
	})

	t.Run("Peer disconnected error", func(t *testing.T) {
		if ErrPeerDisconnected == nil {
			t.Error("ErrPeerDisconnected should not be nil")
		}
	})

	t.Run("Invalid peer config error", func(t *testing.T) {
		if ErrInvalidPeerConfig == nil {
			t.Error("ErrInvalidPeerConfig should not be nil")
		}
	})
}

func TestFrameErrors(t *testing.T) {
	t.Run("Frame too large error", func(t *testing.T) {
		if ErrFrameTooLarge == nil {
			t.Error("ErrFrameTooLarge should not be nil")
		}
	})

	t.Run("Invalid frame header error", func(t *testing.T) {
		if ErrInvalidFrameHeader == nil {
			t.Error("ErrInvalidFrameHeader should not be nil")
		}
	})

	t.Run("Invalid frame payload error", func(t *testing.T) {
		if ErrInvalidFramePayload == nil {
			t.Error("ErrInvalidFramePayload should not be nil")
		}
	})
}
