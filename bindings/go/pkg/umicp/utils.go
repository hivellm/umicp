package umicp

import (
	"time"
)

// GetTimestamp returns the current Unix timestamp in milliseconds
func GetTimestamp() int64 {
	return time.Now().UnixMilli()
}

// GetTimestampMicro returns the current Unix timestamp in microseconds
func GetTimestampMicro() int64 {
	return time.Now().UnixMicro()
}

// IsValidMessageID checks if a message ID is valid (non-empty)
func IsValidMessageID(id string) bool {
	return id != ""
}

// IsValidPeerID checks if a peer ID is valid (non-empty and reasonable length)
func IsValidPeerID(id string) bool {
	return id != "" && len(id) <= 255
}
