package umicp

import (
	"testing"
	"time"
)

func TestGetTimestamp(t *testing.T) {
	ts := GetTimestamp()

	if ts <= 0 {
		t.Error("Timestamp should be positive")
	}

	// Should be in milliseconds (13 digits for current time)
	if ts < 1000000000000 {
		t.Error("Timestamp should be in milliseconds")
	}
}

func TestGetTimestampMicro(t *testing.T) {
	ts := GetTimestampMicro()

	if ts <= 0 {
		t.Error("Timestamp should be positive")
	}

	// Should be in microseconds (16 digits for current time)
	if ts < 1000000000000000 {
		t.Error("Timestamp should be in microseconds")
	}
}

func TestGetTimestamp_Ordering(t *testing.T) {
	ts1 := GetTimestamp()
	time.Sleep(2 * time.Millisecond)
	ts2 := GetTimestamp()

	if ts2 <= ts1 {
		t.Error("Later timestamp should be greater")
	}
}

func TestGetTimestampMicro_Ordering(t *testing.T) {
	ts1 := GetTimestampMicro()
	time.Sleep(1 * time.Millisecond)
	ts2 := GetTimestampMicro()

	if ts2 <= ts1 {
		t.Error("Later timestamp should be greater")
	}
}

func TestIsValidMessageID(t *testing.T) {
	tests := []struct {
		name     string
		id       string
		expected bool
	}{
		{"Valid UUID", "550e8400-e29b-41d4-a716-446655440000", true},
		{"Valid custom ID", "msg-123-abc", true},
		{"Valid numeric", "12345", true},
		{"Empty string", "", false},
		{"Only spaces", "   ", true}, // Function only checks non-empty
		{"Valid with spaces", "msg 123", true},
		{"Single char", "a", true},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := IsValidMessageID(tt.id)
			if result != tt.expected {
				t.Errorf("IsValidMessageID(%q) = %v, want %v", tt.id, result, tt.expected)
			}
		})
	}
}

func TestIsValidPeerID(t *testing.T) {
	tests := []struct {
		name     string
		id       string
		expected bool
	}{
		{"Valid UUID", "550e8400-e29b-41d4-a716-446655440000", true},
		{"Valid peer ID", "peer-123", true},
		{"Valid numeric", "12345", true},
		{"Empty string", "", false},
		{"Only spaces", "   ", true}, // Function only checks non-empty and length
		{"Valid with underscore", "peer_123", true},
		{"Single char", "p", true},
		{"Valid with dots", "peer.123.abc", true},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := IsValidPeerID(tt.id)
			if result != tt.expected {
				t.Errorf("IsValidPeerID(%q) = %v, want %v", tt.id, result, tt.expected)
			}
		})
	}
}
