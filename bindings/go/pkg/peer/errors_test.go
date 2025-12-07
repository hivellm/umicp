package peer

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestGenericError(t *testing.T) {
	t.Run("Create GenericError", func(t *testing.T) {
		err := &GenericError{
			Message: "connection refused",
		}

		assert.NotNil(t, err)
		assert.Equal(t, "connection refused", err.Message)
	})

	t.Run("GenericError Error() method", func(t *testing.T) {
		err := &GenericError{
			Message: "peer not found",
		}

		errorMsg := err.Error()
		assert.Equal(t, "peer not found", errorMsg)
	})

	t.Run("GenericError with empty message", func(t *testing.T) {
		err := &GenericError{
			Message: "",
		}

		errorMsg := err.Error()
		assert.Equal(t, "", errorMsg)
	})

	t.Run("Multiple GenericError scenarios", func(t *testing.T) {
		testCases := []struct {
			name    string
			message string
		}{
			{
				name:    "Connection timeout",
				message: "timeout after 30s",
			},
			{
				name:    "Message send failure",
				message: "connection closed",
			},
			{
				name:    "Handshake failure",
				message: "protocol mismatch",
			},
			{
				name:    "Peer not found",
				message: "peer with ID 'test-123' not found",
			},
			{
				name:    "Network error",
				message: "network unreachable",
			},
		}

		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				err := &GenericError{
					Message: tc.message,
				}

				assert.Equal(t, tc.message, err.Error())
			})
		}
	})

	t.Run("GenericError implements error interface", func(t *testing.T) {
		var err error = &GenericError{
			Message: "test error",
		}

		assert.EqualError(t, err, "test error")
	})
}
