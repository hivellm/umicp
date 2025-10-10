"""Tests for error module."""

import pytest

from umicp.error import (
    UmicpError,
    ValidationError,
    SerializationError,
    TransportError,
    MatrixOperationError,
    ConnectionError,
    TimeoutError,
)


class TestUmicpError:
    """Test UmicpError base class."""

    def test_create_error(self):
        """Test creating base error."""
        error = UmicpError("Test error")
        assert str(error) == "Test error"
        assert error.message == "Test error"

    def test_error_with_details(self):
        """Test error with details."""
        error = UmicpError("Test error", details={"code": 123})
        assert "Test error" in str(error)
        assert error.details["code"] == 123


class TestValidationError:
    """Test ValidationError."""

    def test_validation_error(self):
        """Test validation error."""
        error = ValidationError("Invalid field")
        assert isinstance(error, UmicpError)
        assert str(error) == "Invalid field"


class TestSerializationError:
    """Test SerializationError."""

    def test_serialization_error(self):
        """Test serialization error."""
        error = SerializationError("Failed to serialize")
        assert isinstance(error, UmicpError)
        assert "serialize" in str(error)


class TestTransportError:
    """Test TransportError."""

    def test_transport_error(self):
        """Test transport error."""
        error = TransportError("Connection failed")
        assert isinstance(error, UmicpError)
        assert "failed" in str(error)


class TestMatrixOperationError:
    """Test MatrixOperationError."""

    def test_matrix_error(self):
        """Test matrix operation error."""
        error = MatrixOperationError("Invalid dimensions")
        assert isinstance(error, UmicpError)
        assert "dimensions" in str(error)


class TestConnectionError:
    """Test ConnectionError."""

    def test_connection_error(self):
        """Test connection error."""
        error = ConnectionError("Cannot connect")
        assert isinstance(error, UmicpError)
        assert "connect" in str(error)


class TestTimeoutError:
    """Test TimeoutError."""

    def test_timeout_error(self):
        """Test timeout error."""
        error = TimeoutError("Operation timed out")
        assert isinstance(error, UmicpError)
        assert "timed out" in str(error)

