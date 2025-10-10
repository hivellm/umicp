"""Tests for service discovery module."""

import pytest
import asyncio
from datetime import datetime

from umicp.discovery import ServiceDiscovery, ServiceInfo


class TestServiceInfo:
    """Test ServiceInfo dataclass."""

    def test_create_service_info(self):
        """Test creating service info."""
        info = ServiceInfo(
            id="service-001",
            name="TestService",
            version="1.0.0",
            capabilities={"type": "processor"},
            metadata={"region": "us-east-1"}
        )

        assert info.id == "service-001"
        assert info.name == "TestService"
        assert info.version == "1.0.0"
        assert info.capabilities["type"] == "processor"
        assert info.metadata["region"] == "us-east-1"
        assert info.health_status == "healthy"


class TestServiceDiscovery:
    """Test ServiceDiscovery class."""

    @pytest.mark.asyncio
    async def test_create_discovery(self):
        """Test creating service discovery."""
        discovery = ServiceDiscovery()
        assert discovery is not None

    @pytest.mark.asyncio
    async def test_register_service(self):
        """Test registering service."""
        discovery = ServiceDiscovery()

        service = ServiceInfo(
            id="service-001",
            name="TestService",
            version="1.0.0"
        )

        await discovery.register(service)

        found = await discovery.find_by_id("service-001")
        assert found is not None
        assert found.id == "service-001"
        assert found.name == "TestService"

    @pytest.mark.asyncio
    async def test_unregister_service(self):
        """Test unregistering service."""
        discovery = ServiceDiscovery()

        service = ServiceInfo(id="service-001", name="Test", version="1.0.0")
        await discovery.register(service)

        result = await discovery.unregister("service-001")
        assert result is True

        found = await discovery.find_by_id("service-001")
        assert found is None

    @pytest.mark.asyncio
    async def test_find_by_capability(self):
        """Test finding services by capability."""
        discovery = ServiceDiscovery()

        service1 = ServiceInfo(
            id="service-001",
            name="Service1",
            version="1.0.0",
            capabilities={"type": "processor", "language": "python"}
        )

        service2 = ServiceInfo(
            id="service-002",
            name="Service2",
            version="1.0.0",
            capabilities={"type": "storage", "language": "go"}
        )

        await discovery.register(service1)
        await discovery.register(service2)

        processors = await discovery.find_by_capability("type", "processor")
        assert len(processors) == 1
        assert processors[0].id == "service-001"

        python_services = await discovery.find_by_capability("language", "python")
        assert len(python_services) == 1
        assert python_services[0].id == "service-001"

    @pytest.mark.asyncio
    async def test_list_all_services(self):
        """Test listing all services."""
        discovery = ServiceDiscovery()

        service1 = ServiceInfo(id="service-001", name="Test1", version="1.0.0")
        service2 = ServiceInfo(id="service-002", name="Test2", version="1.0.0")

        await discovery.register(service1)
        await discovery.register(service2)

        all_services = await discovery.list_all()
        assert len(all_services) == 2

    @pytest.mark.asyncio
    async def test_update_health(self):
        """Test updating service health."""
        discovery = ServiceDiscovery()

        service = ServiceInfo(id="service-001", name="Test", version="1.0.0")
        await discovery.register(service)

        result = await discovery.update_health("service-001", "unhealthy")
        assert result is True

        found = await discovery.find_by_id("service-001")
        assert found.health_status == "unhealthy"

    @pytest.mark.asyncio
    async def test_start_stop(self):
        """Test starting and stopping discovery."""
        discovery = ServiceDiscovery()

        await discovery.start()
        assert discovery._cleanup_task is not None

        await discovery.stop()
        assert discovery._cleanup_task is None or discovery._cleanup_task.cancelled()

