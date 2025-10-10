"""Pytest configuration and fixtures."""

import pytest
import asyncio


@pytest.fixture(scope="session")
def event_loop():
    """Create event loop for async tests."""
    loop = asyncio.get_event_loop_policy().new_event_loop()
    yield loop
    loop.close()


@pytest.fixture
def sample_envelope_data():
    """Sample envelope data for testing."""
    return {
        "from": "client-001",
        "to": "server-001",
        "operation": "data",
        "message_id": "msg-12345",
        "timestamp": "2025-10-10T12:00:00Z",
        "capabilities": {
            "content-type": "application/json",
            "version": "1.0"
        }
    }


@pytest.fixture
def sample_service_info():
    """Sample service info for testing."""
    from umicp.discovery import ServiceInfo
    return ServiceInfo(
        id="test-service",
        name="TestService",
        version="1.0.0",
        capabilities={"type": "test"},
        metadata={"environment": "test"}
    )

