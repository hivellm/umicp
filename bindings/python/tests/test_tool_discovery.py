"""Tests for UMICP tool discovery (v0.2.0)."""

import pytest
from umicp.tool_discovery import (
    OperationSchema,
    ServerInfo,
    DiscoverableService,
    generate_operations_response,
    generate_schema_response,
    generate_server_info_response,
)


# Example service for testing
class TestService:
    """Test service implementing DiscoverableService protocol."""

    def list_operations(self):
        return [
            OperationSchema(
                name="search_vectors",
                title="Search Vectors",
                description="Search for semantically similar content",
                input_schema={
                    "type": "object",
                    "properties": {
                        "collection": {"type": "string"},
                        "query": {"type": "string"},
                        "limit": {"type": "integer", "default": 10}
                    },
                    "required": ["collection", "query"]
                },
                annotations={"read_only": True, "idempotent": True}
            ),
            OperationSchema(
                name="create_collection",
                title="Create Collection",
                input_schema={
                    "type": "object",
                    "properties": {
                        "name": {"type": "string"},
                        "dimension": {"type": "integer"}
                    },
                    "required": ["name", "dimension"]
                },
                annotations={"read_only": False}
            )
        ]

    def get_schema(self, name: str):
        for op in self.list_operations():
            if op.name == name:
                return op
        return None

    def server_info(self):
        return ServerInfo(
            server="test-service",
            version="1.0.0",
            protocol="UMICP/0.2",
            features=["discovery", "search"],
            operations_count=2,
            mcp_compatible=True
        )


def test_operation_schema_creation():
    """Test OperationSchema creation."""
    schema = OperationSchema(
        name="test_op",
        input_schema={"type": "object"},
        title="Test Operation",
        description="A test operation",
        annotations={"read_only": True}
    )

    assert schema.name == "test_op"
    assert schema.title == "Test Operation"
    assert schema.description == "A test operation"
    assert schema.input_schema == {"type": "object"}
    assert schema.annotations == {"read_only": True}


def test_operation_schema_to_dict():
    """Test OperationSchema serialization."""
    schema = OperationSchema(
        name="search",
        title="Search",
        input_schema={"type": "object", "properties": {"query": {"type": "string"}}},
        annotations={"read_only": True}
    )

    result = schema.to_dict()
    assert result["name"] == "search"
    assert result["title"] == "Search"
    assert "input_schema" in result
    assert result["annotations"]["read_only"] is True


def test_server_info_creation():
    """Test ServerInfo creation."""
    info = ServerInfo(
        server="my-service",
        version="1.0.0",
        protocol="UMICP/0.2",
        features=["discovery", "streaming"],
        operations_count=10,
        mcp_compatible=True
    )

    assert info.server == "my-service"
    assert info.version == "1.0.0"
    assert info.protocol == "UMICP/0.2"
    assert len(info.features) == 2
    assert info.operations_count == 10
    assert info.mcp_compatible is True


def test_server_info_to_dict():
    """Test ServerInfo serialization."""
    info = ServerInfo(
        server="vectorizer",
        version="0.9.0",
        protocol="UMICP/0.2",
        features=["discovery"],
        mcp_compatible=True,
        metadata={"license": "MIT"}
    )

    result = info.to_dict()
    assert result["server"] == "vectorizer"
    assert result["features"] == ["discovery"]
    assert result["metadata"]["license"] == "MIT"


def test_discoverable_service_list_operations():
    """Test DiscoverableService.list_operations()."""
    service = TestService()
    operations = service.list_operations()

    assert len(operations) == 2
    assert operations[0].name == "search_vectors"
    assert operations[1].name == "create_collection"


def test_discoverable_service_get_schema_found():
    """Test DiscoverableService.get_schema() - operation found."""
    service = TestService()
    schema = service.get_schema("search_vectors")

    assert schema is not None
    assert schema.name == "search_vectors"
    assert schema.title == "Search Vectors"
    assert schema.annotations["read_only"] is True


def test_discoverable_service_get_schema_not_found():
    """Test DiscoverableService.get_schema() - operation not found."""
    service = TestService()
    schema = service.get_schema("non_existent")

    assert schema is None


def test_discoverable_service_server_info():
    """Test DiscoverableService.server_info()."""
    service = TestService()
    info = service.server_info()

    assert info.server == "test-service"
    assert info.version == "1.0.0"
    assert info.protocol == "UMICP/0.2"
    assert "discovery" in info.features
    assert info.mcp_compatible is True


def test_generate_operations_response():
    """Test generate_operations_response() helper."""
    service = TestService()
    response = generate_operations_response(service)

    assert "operations" in response
    assert isinstance(response["operations"], list)
    assert len(response["operations"]) == 2
    assert response["count"] == 2
    assert response["protocol"] == "UMICP/0.2"
    assert response["mcp_compatible"] is True


def test_generate_schema_response_found():
    """Test generate_schema_response() - operation found."""
    service = TestService()
    response = generate_schema_response(service, "search_vectors")

    assert "name" in response
    assert response["name"] == "search_vectors"
    assert "input_schema" in response
    assert "error" not in response


def test_generate_schema_response_not_found():
    """Test generate_schema_response() - operation not found."""
    service = TestService()
    response = generate_schema_response(service, "invalid")

    assert "error" in response
    assert response["error"] == "Operation not found"
    assert response["operation"] == "invalid"


def test_generate_server_info_response():
    """Test generate_server_info_response() helper."""
    service = TestService()
    response = generate_server_info_response(service)

    assert response["server"] == "test-service"
    assert response["version"] == "1.0.0"
    assert "features" in response
    assert response["mcp_compatible"] is True


def test_operation_schema_minimal():
    """Test OperationSchema with minimal fields."""
    schema = OperationSchema(
        name="minimal",
        input_schema={"type": "object"}
    )

    result = schema.to_dict()
    assert result["name"] == "minimal"
    assert "input_schema" in result
    assert "title" not in result
    assert "description" not in result


def test_server_info_minimal():
    """Test ServerInfo with minimal fields."""
    info = ServerInfo(
        server="minimal",
        version="1.0",
        protocol="UMICP/0.2"
    )

    result = info.to_dict()
    assert result["server"] == "minimal"
    assert result["version"] == "1.0"
    assert result["protocol"] == "UMICP/0.2"
    assert "features" not in result
    assert "operations_count" not in result

