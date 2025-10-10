"""Tests for Envelope module."""

import pytest
from datetime import datetime

from umicp import Envelope, EnvelopeBuilder, OperationType, ValidationError, SerializationError


class TestEnvelope:
    """Test Envelope class."""

    def test_create_envelope(self):
        """Test creating envelope."""
        envelope = Envelope(
            from_id="client-001",
            to_id="server-001",
            operation=OperationType.DATA,
            message_id="msg-001"
        )

        assert envelope.from_id == "client-001"
        assert envelope.to_id == "server-001"
        assert envelope.operation == OperationType.DATA
        assert envelope.message_id == "msg-001"

    def test_envelope_builder(self):
        """Test envelope builder."""
        envelope = EnvelopeBuilder() \
            .from_id("client-001") \
            .to_id("server-001") \
            .operation(OperationType.DATA) \
            .capability("test", "value") \
            .build()

        assert envelope.from_id == "client-001"
        assert envelope.capabilities["test"] == "value"
        assert envelope.hash_value is not None

    def test_envelope_serialization(self):
        """Test envelope JSON serialization."""
        envelope = EnvelopeBuilder() \
            .from_id("client-001") \
            .to_id("server-001") \
            .operation(OperationType.DATA) \
            .build()

        json_str = envelope.to_json()
        assert isinstance(json_str, str)
        assert "client-001" in json_str

        # Deserialize
        received = Envelope.from_json(json_str)
        assert received.from_id == envelope.from_id
        assert received.to_id == envelope.to_id

    def test_envelope_validation(self):
        """Test envelope validation."""
        with pytest.raises(ValidationError):
            Envelope(
                from_id="",  # Empty from_id should fail
                to_id="server-001",
                operation=OperationType.DATA
            )

    def test_envelope_hash(self):
        """Test envelope hash generation."""
        envelope = EnvelopeBuilder() \
            .from_id("client-001") \
            .to_id("server-001") \
            .operation(OperationType.DATA) \
            .build()

        hash1 = envelope.compute_hash()
        hash2 = envelope.compute_hash()

        assert hash1 == hash2  # Same envelope, same hash
        assert len(hash1) == 64  # SHA-256 hex length

