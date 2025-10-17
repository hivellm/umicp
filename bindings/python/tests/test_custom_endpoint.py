"""
Test for Custom Endpoint Support (v0.2.2)

Verifies that the Python UMICP implementation supports
custom endpoint paths for compatibility with different servers
(e.g., Vectorizer uses /umicp, standard servers use /message)
"""

import pytest
from umicp.transport.http_client import HttpClient
from umicp.envelope import Envelope
from umicp.types import OperationType


class TestCustomEndpoint:
    """Test custom endpoint configuration"""

    def test_client_with_default_path(self):
        """Test that HttpClient uses default path /message"""
        client = HttpClient("http://localhost:8000")
        assert client.base_url == "http://localhost:8000"
        assert client.path == "/message"

    def test_client_with_custom_path_vectorizer(self):
        """Test HttpClient with Vectorizer endpoint /umicp"""
        client = HttpClient("http://localhost:8000", path="/umicp")
        assert client.base_url == "http://localhost:8000"
        assert client.path == "/umicp"

    def test_client_with_custom_path_standard(self):
        """Test HttpClient with standard endpoint /message"""
        client = HttpClient("http://localhost:9000", path="/message")
        assert client.base_url == "http://localhost:9000"
        assert client.path == "/message"

    def test_client_with_trailing_slash_in_base_url(self):
        """Test that trailing slash in base_url works correctly"""
        client = HttpClient("http://localhost:8000/", path="/umicp")
        assert client.base_url == "http://localhost:8000/"
        assert client.path == "/umicp"

    def test_client_with_custom_endpoint_without_leading_slash(self):
        """Test custom endpoint without leading slash"""
        client = HttpClient("http://localhost:8000", path="umicp")
        assert client.path == "umicp"

    def test_multiple_clients_different_endpoints(self):
        """Test creating multiple clients with different endpoints"""
        vectorizer_client = HttpClient("http://localhost:8000", path="/umicp")
        standard_client = HttpClient("http://localhost:9000", path="/message")

        # Both clients should be independent
        assert vectorizer_client.base_url != standard_client.base_url
        assert vectorizer_client.path != standard_client.path
        assert vectorizer_client.path == "/umicp"
        assert standard_client.path == "/message"

    def test_client_path_is_stored_correctly(self):
        """Test that path is stored and accessible"""
        client = HttpClient("http://localhost:8000", path="/custom")
        assert hasattr(client, 'path')
        assert client.path == "/custom"

    def test_default_path_value(self):
        """Test that default path is /message when not specified"""
        client1 = HttpClient("http://localhost:8000")
        client2 = HttpClient("http://localhost:8000", path="/message")

        assert client1.path == client2.path
        assert client1.path == "/message"

    def test_vectorizer_configuration(self):
        """Test typical Vectorizer configuration"""
        # Vectorizer service configuration
        client = HttpClient(
            base_url="http://localhost:8000",
            path="/umicp"
        )

        assert client.base_url == "http://localhost:8000"
        assert client.path == "/umicp"

    def test_version_0_2_2_supports_custom_endpoints(self):
        """Test that v0.2.2 supports custom endpoints"""
        # This test ensures that v0.2.2 supports custom endpoints
        # The functionality is verified through the other tests

        # Test both old (default) and new (custom) methods work
        client1 = HttpClient("http://localhost:8000")  # Default
        client2 = HttpClient("http://localhost:8000", path="/umicp")  # Custom

        assert client1.path == "/message"
        assert client2.path == "/umicp"


class TestCustomEndpointBackwardCompatibility:
    """Test backward compatibility of custom endpoint feature"""

    def test_old_code_still_works(self):
        """Test that old code without path parameter still works"""
        # Old code that doesn't specify path should still work
        client = HttpClient("http://localhost:8000")

        assert client.base_url == "http://localhost:8000"
        # Should use default path
        assert client.path == "/message"

    def test_explicit_default_path(self):
        """Test explicitly setting default path"""
        client = HttpClient("http://localhost:8000", path="/message")

        assert client.path == "/message"


class TestCustomEndpointEdgeCases:
    """Test edge cases for custom endpoint configuration"""

    def test_empty_path(self):
        """Test with empty path"""
        client = HttpClient("http://localhost:8000", path="")
        assert client.path == ""

    def test_path_with_query_params(self):
        """Test path with query parameters (edge case)"""
        client = HttpClient("http://localhost:8000", path="/umicp?version=1")
        assert client.path == "/umicp?version=1"

    def test_complex_path(self):
        """Test with complex path"""
        client = HttpClient("http://localhost:8000", path="/api/v1/umicp")
        assert client.path == "/api/v1/umicp"

